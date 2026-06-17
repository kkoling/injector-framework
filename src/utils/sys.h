#pragma once
#include "utils.h"
#include "vars.h"

EXTERN_C {
  VOID InitSyscall(DWORD ssn, void* syscall_stub);
  NTSTATUS DirectSyscall(...);
  NTSTATUS IndirectSyscall(...);
  NTSTATUS DirectSyscallSSN(DWORD ssn, ...);
  VOID CALLBACK ProxySyscall(PTP_CALLBACK_INSTANCE instance, PVOID context,
                             PTP_WORK work);
}

typedef struct {
  std::string_view Name;
  SIZE_T Hash;
  PVOID Address;
  DWORD SSN;
  PVOID InstOpAddr;
  PVOID RetOpAddr;
} SYSCALL_INFO, *PSYSCALL_INFO;

typedef struct {
  PVOID Address;
  uintptr_t Args[11];
  BYTE ArgCount;
} PROXY_ARGS;

namespace sys {
bool init();

void syscall_proxy(PROXY_ARGS* context);
SYSCALL_INFO* get_syscall_info(size_t name_hash);
void print_syscalls();

static inline std::vector<SYSCALL_INFO> syscall_list;
}  // namespace sys

#define SYSCALL(name_hash, ...)                                      \
  ([&]() -> NTSTATUS {                                               \
    SYSCALL_INFO* info = sys::get_syscall_info(name_hash);           \
    if (!info) return STATUS_NOT_FOUND;                              \
    InitSyscall(info->SSN, info->InstOpAddr);                        \
                                                                     \
    if (vars::syscall_indirect) return IndirectSyscall(__VA_ARGS__); \
                                                                     \
    return DirectSyscall(__VA_ARGS__);                               \
  }())

#pragma region FUNCTION WRAPPERS
bool NtReadMem(HANDLE proc_handle, void* base, void* buffer, size_t size,
               size_t* bytes_read = NULL);
bool NtWriteMem(HANDLE proc_handle, void* base, void* buffer, size_t size,
                size_t* bytes_written = NULL);
void* NtAllocMem(HANDLE proc_handle, size_t region_size, ULONG alloc_type,
                 ULONG page_protection);
bool NtFreeMem(HANDLE proc_handle, void* src);
DWORD NtProtectMem(HANDLE proc_handle, void* src, size_t region_size,
                   ULONG new_protection);
void NtCloseHandle(HANDLE handle);
#pragma endregion