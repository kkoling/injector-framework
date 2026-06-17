#include "sys.h"

namespace px {
static TpAllocWork TpAllocWorkFn;
static TpPostWork TpPostWorkFn;
static TpWaitForWork TpWaitForWorkFn;
static TpReleaseWork TpReleaseWorkFn;
}  // namespace px

bool sys::init() {
  void* ntdll = utils::get_ntdll();
  if (!ntdll) return false;

  PBYTE base = (PBYTE)ntdll;
  IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)ntdll;
  IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

  DWORD export_rva =
      nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
          .VirtualAddress;
  IMAGE_EXPORT_DIRECTORY* export_dir =
      (IMAGE_EXPORT_DIRECTORY*)(base + export_rva);

  DWORD* func_table = (DWORD*)(base + export_dir->AddressOfFunctions);
  DWORD* name_table = (DWORD*)(base + export_dir->AddressOfNames);
  WORD* ord_table = (WORD*)(base + export_dir->AddressOfNameOrdinals);

  std::vector<SYSCALL_INFO> temp_list;

  for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
    const char* cur_name = (const char*)(base + name_table[i]);
    if ((cur_name[0] == 'N' && cur_name[1] == 't') ||
        (cur_name[0] == 'Z' && cur_name[1] == 'w')) {
      DWORD func_rva = func_table[ord_table[i]];
      PBYTE func_addr = base + func_rva;

      WORD syscall_number = *(WORD*)(func_addr + 4);
      size_t syscall_hash = utils::fnv1a(cur_name);

      void* sysop = nullptr;
      void* retop = nullptr;
      for (int i = 0; i < 30; i++) {
        if (*(PBYTE)(func_addr + i) == 0x0F &&
            *(PBYTE)(func_addr + i + 1) == 0x05) {
          sysop = (void*)(func_addr + i);
          retop = (void*)(func_addr + 2);
        }
      }

      SYSCALL_INFO info{};
      info.Name = cur_name;
      info.Hash = syscall_hash;
      info.SSN = syscall_number;
      info.Address = (void*)func_addr;
      info.InstOpAddr = sysop;
      info.RetOpAddr = retop;

      temp_list.push_back(info);
    }
  }

  syscall_list = std::move(temp_list);

  constexpr size_t hash_alloc = utils::fnv1a_compile("TpAllocWork");
  constexpr size_t hash_rel = utils::fnv1a_compile("TpReleaseWork");
  constexpr size_t hash_wait = utils::fnv1a_compile("TpWaitForWork");
  constexpr size_t hash_post = utils::fnv1a_compile("TpPostWork");

  px::TpAllocWorkFn =
      (TpAllocWork)utils::get_export_address_hash(ntdll, hash_alloc);
  px::TpPostWorkFn =
      (TpPostWork)utils::get_export_address_hash(ntdll, hash_post);
  px::TpWaitForWorkFn =
      (TpWaitForWork)utils::get_export_address_hash(ntdll, hash_wait);
  px::TpReleaseWorkFn =
      (TpReleaseWork)utils::get_export_address_hash(ntdll, hash_rel);

  return true;
}

SYSCALL_INFO* sys::get_syscall_info(size_t name_hash) {
  if (syscall_list.empty()) return nullptr;
  for (auto& i : syscall_list) {
    if (i.Hash == name_hash) return &i;
  }
  return nullptr;
}

void sys::print_syscalls() {
  if (syscall_list.empty()) return;

  int idx = 0;
  for (const auto& i : syscall_list) {
    printf("[%i] [NAME] %s | [FUNC] %p | [SSN] %lu | [INST] %p | [RET] %p\n",
           idx, i.Name.data(), i.Address, i.SSN, i.InstOpAddr, i.RetOpAddr);
    idx++;
  }
}

void sys::syscall_proxy(PROXY_ARGS* context) {
  PTP_WORK work_ret = NULL;
  px::TpAllocWorkFn(&work_ret, (PTP_WORK_CALLBACK)ProxySyscall, context, NULL);
  px::TpPostWorkFn(work_ret);
  px::TpWaitForWorkFn(work_ret, false);
  px::TpReleaseWorkFn(work_ret);
}

#pragma region FUNCTION WRAPPERS
bool NtReadMem(HANDLE proc_handle, void* base, void* buffer, size_t size,
               size_t* bytes_read) {
  constexpr size_t hash = utils::fnv1a_compile("NtReadVirtualMemory");

  if (vars::proxy_call) {
    static void* fn_addr =
        utils::get_export_address_hash(utils::get_ntdll(), hash);
    PROXY_ARGS args{};
    args.Address = fn_addr;
    args.Args[0] = (uintptr_t)proc_handle;
    args.Args[1] = (uintptr_t)base;
    args.Args[2] = (uintptr_t)buffer;
    args.Args[3] = (uintptr_t)size;
    args.Args[4] = (uintptr_t)bytes_read;
    args.ArgCount = 5;

    sys::syscall_proxy(&args);
  } else {
    NTSTATUS status =
        SYSCALL(hash, proc_handle, base, buffer, size, bytes_read);
    if (!NT_SUCCESS(status)) {
      DBG("[read] failed %p: 0x%lX\n", base, status);
      return false;
    }
  }
  return true;
}

bool NtWriteMem(HANDLE proc_handle, void* base, void* buffer, size_t size,
                size_t* bytes_written) {
  constexpr size_t hash = utils::fnv1a_compile("NtWriteVirtualMemory");

  if (vars::proxy_call) {
    static void* fn_addr =
        utils::get_export_address_hash(utils::get_ntdll(), hash);
    PROXY_ARGS args{};
    args.Address = fn_addr;
    args.Args[0] = (uintptr_t)proc_handle;
    args.Args[1] = (uintptr_t)base;
    args.Args[2] = (uintptr_t)buffer;
    args.Args[3] = (uintptr_t)size;
    args.Args[4] = (uintptr_t)bytes_written;
    args.ArgCount = 5;

    sys::syscall_proxy(&args);
  } else {
    NTSTATUS status =
        SYSCALL(hash, proc_handle, base, buffer, size, bytes_written);
    if (!NT_SUCCESS(status)) {
      DBG("[write] failed %p: 0x%lX\n", base, status);
      return false;
    }
  }

  return true;
}

void* NtAllocMem(HANDLE proc_handle, size_t region_size, ULONG alloc_type,
                 ULONG page_protection) {
  void* alloc_addr = nullptr;
  size_t alloc_size = region_size;
  constexpr size_t hash = utils::fnv1a_compile("NtAllocateVirtualMemory");

  if (vars::proxy_call) {
    static void* fn_addr =
        utils::get_export_address_hash(utils::get_ntdll(), hash);
    PROXY_ARGS args{};
    args.Address = fn_addr;
    args.Args[0] = (uintptr_t)proc_handle;
    args.Args[1] = (uintptr_t)&alloc_addr;
    args.Args[2] = (uintptr_t)0;
    args.Args[3] = (uintptr_t)&alloc_size;
    args.Args[4] = (uintptr_t)alloc_type;
    args.Args[5] = (uintptr_t)page_protection;
    args.ArgCount = 6;

    sys::syscall_proxy(&args);
  } else {
    NTSTATUS status = SYSCALL(hash, proc_handle, &alloc_addr, 0, &alloc_size,
                              alloc_type, page_protection);
    if (!NT_SUCCESS(status)) {
      DBG("[alloc] failed 0x%lX\n", status);
      return nullptr;
    }
  }
  return alloc_addr;
}

bool NtFreeMem(HANDLE proc_handle, void* src) {
  void* alloc_addr = src;
  constexpr size_t hash = utils::fnv1a_compile("NtFreeVirtualMemory");

  if (vars::proxy_call) {
    static void* fn_addr =
        utils::get_export_address_hash(utils::get_ntdll(), hash);
    PROXY_ARGS args{};
    args.Address = fn_addr;
    args.Args[0] = (uintptr_t)proc_handle;
    args.Args[1] = (uintptr_t)&alloc_addr;
    args.Args[2] = (uintptr_t)0;
    args.Args[3] = (uintptr_t)MEM_RELEASE;
    args.ArgCount = 4;

    sys::syscall_proxy(&args);
  } else {
    NTSTATUS status = SYSCALL(hash, proc_handle, &alloc_addr, 0, MEM_RELEASE);
    if (!NT_SUCCESS(status)) {
      DBG("[free] failed 0x%lX\n", status);
      return false;
    }
  }
  return true;
}

DWORD NtProtectMem(HANDLE proc_handle, void* src, size_t region_size,
                   ULONG new_protection) {
  void* alloc_addr = src;
  size_t alloc_size = region_size;
  ULONG oldp = 0;
  constexpr size_t hash = utils::fnv1a_compile("NtProtectVirtualMemory");

  if (vars::proxy_call) {
    static void* fn_addr =
        utils::get_export_address_hash(utils::get_ntdll(), hash);
    PROXY_ARGS args{};
    args.Address = fn_addr;
    args.Args[0] = (uintptr_t)proc_handle;
    args.Args[1] = (uintptr_t)&alloc_addr;
    args.Args[2] = (uintptr_t)&alloc_size;
    args.Args[3] = (uintptr_t)new_protection;
    args.Args[4] = (uintptr_t)&oldp;
    args.ArgCount = 5;

    sys::syscall_proxy(&args);

    return oldp;
  } else {
    NTSTATUS status = SYSCALL(hash, proc_handle, &alloc_addr, &alloc_size,
                              new_protection, &oldp);
    if (!NT_SUCCESS(status)) {
      DBG("[protect] failed 0x%lX\n", status);
      return 0;
    }

    return oldp;
  }

  return 0;
}

void NtCloseHandle(HANDLE handle) {
  constexpr size_t hash = utils::fnv1a_compile("NtClose");

  if (vars::proxy_call) {
    static void* fn_addr =
        utils::get_export_address_hash(utils::get_ntdll(), hash);
    PROXY_ARGS args{};
    args.Address = fn_addr;
    args.Args[0] = (uintptr_t)handle;
    args.ArgCount = 1;

    sys::syscall_proxy(&args);
  } else {
    NTSTATUS status = SYSCALL(hash, handle);
    if (!NT_SUCCESS(status)) {
      DBG("[close] failed 0x%lX\n", status);
      return;
    }
  }
}
#pragma endregion