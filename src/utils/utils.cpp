#include "utils.h"

void* utils::get_ntdll() {
  size_t peb = __readgsqword(0x60);
  size_t ldr = *(size_t*)(peb + 0x18);
  size_t list = *(size_t*)(ldr + 0x10);
  size_t ldr_entry = *(size_t*)list;
  return *(void**)(ldr_entry + 0x30);
}

void* utils::get_module_address(const char* module_name) {
  wchar_t wmodule_name[MAX_PATH]{};
  size_t len = strlen(module_name);
  for (size_t i = 0; i < len; i++)
    wmodule_name[i] = (wchar_t)(unsigned char)module_name[i];

  wmodule_name[len] = L'\0';

  return get_module_address_w(wmodule_name);
}

void* utils::get_module_address_w(const wchar_t* module_name) {
  PPEB peb = (PPEB)__readgsqword(0x60);

  PLIST_ENTRY entry = peb->Ldr->InLoadOrderModuleList.Flink;
  PLIST_ENTRY end = &peb->Ldr->InLoadOrderModuleList;
  while (entry != end) {
    PLDR_DATA_TABLE_ENTRY table =
        CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

    if (wcslen(module_name) == 0)
      DBG("%ws\n", table->BaseDllName.Buffer);

    else if (wcscmp(table->BaseDllName.Buffer, module_name) == 0)
      return table->DllBase;

    entry = entry->Flink;
  }

  return nullptr;
}

void* utils::get_module_address_hash(size_t hash) {
  PPEB peb = (PPEB)__readgsqword(0x60);

  PLIST_ENTRY entry = peb->Ldr->InLoadOrderModuleList.Flink;
  PLIST_ENTRY end = &peb->Ldr->InLoadOrderModuleList;
  while (entry != end) {
    PLDR_DATA_TABLE_ENTRY table =
        CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

    size_t cur_hash = fnv1w(table->BaseDllName.Buffer);
    if (cur_hash == hash) return table->DllBase;

    entry = entry->Flink;
  }

  return nullptr;
}

void* utils::get_export_address(void* module_addr, const char* export_name) {
  PBYTE base = (PBYTE)module_addr;
  IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)module_addr;
  IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

  DWORD export_rva =
      nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
          .VirtualAddress;

  IMAGE_EXPORT_DIRECTORY* export_dir =
      (IMAGE_EXPORT_DIRECTORY*)(base + export_rva);

  DWORD* func_table = (DWORD*)(base + export_dir->AddressOfFunctions);
  DWORD* name_table = (DWORD*)(base + export_dir->AddressOfNames);
  WORD* ord_table = (WORD*)(base + export_dir->AddressOfNameOrdinals);
  DWORD count = 0;

  if (strlen(export_name) == 0) {
    for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
      const char* cur_name = (const char*)(base + name_table[i]);
      if (cur_name[0] == 'N' && cur_name[1] == 't') {
        DBG("[%lu] %s\n", count, cur_name);
        count++;
      }
    }

    return nullptr;
  }

  else {
    for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
      const char* cur_name = (const char*)(base + name_table[i]);
      if (_stricmp(cur_name, export_name) == 0)
        return (void*)(base + func_table[ord_table[i]]);
    }
  }
  return nullptr;
}

void* utils::get_export_address_hash(void* module_addr,
                                     size_t export_name_hash) {
  PBYTE base = (PBYTE)module_addr;
  IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)module_addr;
  IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

  DWORD export_rva =
      nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
          .VirtualAddress;

  IMAGE_EXPORT_DIRECTORY* export_dir =
      (IMAGE_EXPORT_DIRECTORY*)(base + export_rva);

  DWORD* func_table = (DWORD*)(base + export_dir->AddressOfFunctions);
  DWORD* name_table = (DWORD*)(base + export_dir->AddressOfNames);
  WORD* ord_table = (WORD*)(base + export_dir->AddressOfNameOrdinals);

  for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
    const char* cur_name = (const char*)(base + name_table[i]);
    if (utils::fnv1a(cur_name) == export_name_hash)
      return (void*)(base + func_table[ord_table[i]]);
  }

  return nullptr;
}

bool utils::enable_admin_privilege() {
  bool is_elevated = FALSE;
  HANDLE token = NULL;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
    TOKEN_ELEVATION elevation;
    DWORD size;
    if (GetTokenInformation(token, TokenElevation, &elevation,
                            sizeof(elevation), &size))
      is_elevated = elevation.TokenIsElevated;
    CloseHandle(token);
  }
  if (is_elevated) return true;

  char path[MAX_PATH];
  if (!GetModuleFileNameA(NULL, path, MAX_PATH)) return false;

  SHELLEXECUTEINFOA sei = {sizeof(sei)};
  sei.lpVerb = "runas";
  sei.lpFile = path;
  sei.nShow = SW_SHOWNORMAL;
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;

  if (!ShellExecuteExA(&sei)) return false;

  ExitProcess(0);
}

bool utils::enable_debug_privilege() {
  HANDLE token = NULL;
  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
    return false;

  LUID luid;
  if (!LookupPrivilegeValueA(NULL, SE_DEBUG_NAME, &luid)) {
    CloseHandle(token);
    return false;
  }

  TOKEN_PRIVILEGES tp;
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL)) {
    CloseHandle(token);
    return false;
  }

  CloseHandle(token);
  return true;
}

void utils::chartowchar(char* in_str, wchar_t* out_str, size_t out_len) {
  size_t len = strlen(in_str);
  if (len <= 0) return;

  if (len + 1 > out_len) return;

  for (size_t i = 0; i < len; i++) {
    out_str[i] = (wchar_t)(unsigned char)in_str[i];
  }

  out_str[len] = L'\0';
}

void utils::wchartochar(wchar_t* in_str, char* out_str, size_t out_len) {
  size_t len = wcslen(in_str);
  if (len <= 0) return;

  if (len + 1 > out_len) return;

  for (size_t i = 0; i < len; i++) {
    out_str[i] = (char)(unsigned char)in_str[i];
  }

  out_str[len] = '\0';
}

void utils::rtl_init_unicode_string(UNICODE_STRING* unicode_string,
                                    const wchar_t* wstring) {
  if (wstring)
    unicode_string->MaximumLength =
        (unicode_string->Length =
             (unsigned short)(wcslen(wstring) * sizeof(wchar_t))) +
        sizeof(UNICODE_NULL);
  else
    unicode_string->MaximumLength = unicode_string->Length = 0;

  unicode_string->Buffer = (wchar_t*)wstring;
}

void utils::rtl_empty_unicode_string(UNICODE_STRING* unicode_string) {
  unicode_string->Length = 0;
  unicode_string->MaximumLength = sizeof(UNICODE_STRING);
  unicode_string->Buffer = NULL;

  return;
}

size_t utils::fnv1a(const char* string) {
  size_t hash = FNV1A_64_OFFSET;
  while (*string) {
    hash ^= static_cast<unsigned char>(*string++);
    hash *= FNV1A_64_PRIME;
  }

  return hash;
}

size_t utils::fnv1w(const wchar_t* wstring) {
  size_t hash = FNV1A_64_OFFSET;
  while (*wstring) {
    wchar_t c = *wstring++;
    hash ^= static_cast<unsigned char>(c & 0xFF);
    hash *= FNV1A_64_PRIME;
    hash ^= static_cast<unsigned char>((c >> 8) & 0xFF);
    hash *= FNV1A_64_PRIME;
  }

  return hash;
}

bool utils::name_matches(const std::string& name, const std::string& filter) {
  if (filter.empty()) return true;
  std::string a = name, b = filter;
  auto fn = [](unsigned char c) { return (char)std::tolower(c); };
  std::transform(a.begin(), a.end(), a.begin(), fn);
  std::transform(b.begin(), b.end(), b.begin(), fn);
  return a.find(b) != std::string::npos;
}