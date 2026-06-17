#include "mem.h"

static std::vector<PROCESS_INFO> process_list;
static std::vector<SERVICE_INFO> service_list;
static std::vector<HANDLE_INFO> handle_list;

bool mem::create_remote_thread(HANDLE proc_handle, void* start_address,
                               void* parameter) {
  constexpr size_t hash_create = utils::fnv1a_compile("NtCreateThreadEx");

  HANDLE thread_handle = nullptr;

  NTSTATUS status = SYSCALL(
      hash_create, &thread_handle, THREAD_ALL_ACCESS, NULL, proc_handle,
      (PUSER_THREAD_START_ROUTINE)(start_address), parameter, 0, 0, 0, 0, NULL);
  if (!NT_SUCCESS(status) || !thread_handle) return false;

  WaitForSingleObject(thread_handle, INFINITE);
  DWORD exit_code = 0;
  if (!GetExitCodeThread(thread_handle, &exit_code)) {
    NtCloseHandle(thread_handle);
    return false;
  }

  // DBG("[create_remote_thread] thread created %p\n", start_address);

  NtCloseHandle(thread_handle);
  return true;
}

bool mem::hijack_remote_thread(DWORD pid, void* start_address,
                               void* parameter) {
  if (process_list.empty()) return false;

  PROCESS_INFO* info;
  for (auto& i : process_list) {
    if (i.PID == pid) {
      info = &i;
      break;
    }
  }
  if (!info) return false;

  size_t hash_suspend = utils::fnv1a_compile("NtSuspendThread");
  size_t hash_resume = utils::fnv1a_compile("NtResumeThread");
  size_t hash_get = utils::fnv1a_compile("NtGetContextThread");
  size_t hash_set = utils::fnv1a_compile("NtSetContextThread");

  for (DWORD tid : info->TID) {
    HANDLE thread_handle = open_thread_handle(pid, tid);
    if (!thread_handle) continue;

    NTSTATUS status = SYSCALL(hash_suspend, tid, NULL);
    if (!NT_SUCCESS(status)) {
      // DBG("[hijack_remote_thread] failed NtSuspendThread [tid] %lu : |
      // 0x%lX\n", tid, status);
      NtCloseHandle(thread_handle);
      continue;
    }

    CONTEXT ctx;

    status = SYSCALL(hash_get, tid, &ctx);
    if (!NT_SUCCESS(status)) {
      // DBG("[hijack_remote_thread] failed NtGetContextThread [tid] %lu : | "
      //"0x%lX\n",
      // tid, status);
      status = SYSCALL(hash_resume, tid, NULL);
      NtCloseHandle(thread_handle);
      continue;
    }

    ctx.ContextFlags = CONTEXT_FULL;
    ctx.Rip = (DWORD64)start_address;

    status = SYSCALL(hash_set, tid, &ctx);
    if (!NT_SUCCESS(status)) {
      // DBG("[hijack_remote_thread] failed NtSetContextThread [tid] %lu : | "
      //  "0x%lX\n",
      //  tid, status);
      status = SYSCALL(hash_resume, tid, NULL);
      NtCloseHandle(thread_handle);
      continue;
    }

    SYSCALL(hash_resume, tid, NULL);
    return true;
  }

  DBG("[hijack_remote_thread] failed to hijack remote thread\n");
  return false;
}

HANDLE mem::open_proc_handle(DWORD pid, DWORD access) {
  constexpr size_t hash = utils::fnv1a_compile("NtOpenProcess");

  HANDLE proc_handle;
  OBJECT_ATTRIBUTES obj{};
  CLIENT_ID id{};

  id.UniqueProcess = (HANDLE)(ULONG_PTR)pid;
  id.UniqueThread = (HANDLE)0;

  InitializeObjectAttributes(&obj, NULL, 0, NULL, NULL);

  NTSTATUS status = SYSCALL(hash, &proc_handle, access, &obj, &id);
  if (!NT_SUCCESS(status)) {
    DBG("[open_proc_handle] failed to open process %lu: 0x%lX\n", pid, status);
    return nullptr;
  }

  return proc_handle;
}

HANDLE mem::open_thread_handle(DWORD pid, DWORD tid, DWORD access) {
  constexpr size_t hash = utils::fnv1a_compile("NtOpenThread");

  HANDLE thread_handle;
  OBJECT_ATTRIBUTES obj{};
  CLIENT_ID id{};

  id.UniqueProcess = (HANDLE)(ULONG_PTR)pid;
  id.UniqueThread = (HANDLE)(ULONG_PTR)tid;

  InitializeObjectAttributes(&obj, NULL, 0, NULL, NULL);

  NTSTATUS status = SYSCALL(hash, &thread_handle, access, &obj, &id);
  if (!NT_SUCCESS(status)) {
    DBG("[open_thread_handle] failed to open process %lu: 0x%lX\n", pid,
        status);
    return nullptr;
  }

  return thread_handle;
}

HANDLE mem::dupe_handle(DWORD pid_source, DWORD pid_target) {
  if (handle_list.empty()) iterate_handles();

  HANDLE proc_handle = open_proc_handle(pid_source, PROCESS_DUP_HANDLE);
  if (!proc_handle) {
    DBG("[dupe_handle] failed to open process %lu\n", pid_source);
    return nullptr;
  }

  NTSTATUS status;

  constexpr size_t hash = utils::fnv1a_compile("NtDuplicateObject");

  for (const auto& i : handle_list) {
    if (i.PID != pid_source) continue;

    HANDLE dupe_handle = nullptr;

    status = SYSCALL(hash, proc_handle, i.HandleValue, NtCurrentProcess(),
                     &dupe_handle, PROCESS_ALL_ACCESS, 0, 0);
    if (!NT_SUCCESS(status)) {
      // DBG("[dupe_handle] failed to duplicate handle: 0x%lX\n", status);
      continue;
    }

    DWORD pid_temp = pid_from_handle(dupe_handle);

    if (pid_temp != pid_target) {
      // DBG("[dupe_handle] duplicated handle does not belong to desired "
      //  "process\n");
      NtCloseHandle(dupe_handle);
      continue;
    }

    NtCloseHandle(proc_handle);
    return dupe_handle;
  }

  DBG("[dupe_handle] failed to duplicate handle\n");
  NtCloseHandle(proc_handle);
  return nullptr;
}

DWORD mem::pid_from_handle(HANDLE handle) {
  PROCESS_BASIC_INFORMATION pbi{};
  constexpr size_t hash = utils::fnv1a_compile("NtQueryInformationProcess");
  NTSTATUS status =
      SYSCALL(hash, handle, ProcessBasicInformation, &pbi, sizeof(pbi), 0);
  if (!NT_SUCCESS(status)) {
    DBG("[pid_from_handle] failed to query process info: 0x%lX\n", status);
    return 0;
  }

  return (DWORD)(ULONG_PTR)pbi.UniqueProcessId;
}

DWORD mem::get_process_id(std::string_view name) {
  if (process_list.empty()) {
    iterate_processes();
  }

  for (const auto& i : process_list) {
    if (i.ProcessName.c_str() == name) {
      return i.PID;
    }
  }

  return 0;
}

DWORD mem::get_service_id(std::string_view name) {
  if (service_list.empty()) {
    iterate_services();
  }

  for (const auto& i : service_list) {
    if (i.ServiceName.data() == name) {
      return i.PID;
    }
  }

  return 0;
}

void mem::iterate_processes() {
  constexpr size_t hash = utils::fnv1a_compile("NtQuerySystemInformation");

  // Call the ntapi to get the ReturnLength value first but the ntapi
  // sometimes doesnt return the value. I should use this as a fallback.
  NTSTATUS status;
  ULONG ret_len = 0x10000;
  void* buffer = NULL;
  buffer = HeapAlloc(GetProcessHeap(), 0, ret_len);
  if (!buffer) return;

  status = SYSCALL(hash, SystemProcessInformation, buffer, ret_len, &ret_len);
  while (status == STATUS_INFO_LENGTH_MISMATCH) {
    ret_len *= 2;

    void* temp = HeapReAlloc(GetProcessHeap(), 0, buffer, ret_len);
    if (!temp) {
      HeapFree(GetProcessHeap(), 0, buffer);
      return;
    }

    buffer = temp;
    status = SYSCALL(hash, SystemProcessInformation, buffer, ret_len, &ret_len);
  }

  if (!NT_SUCCESS(status)) {
    DBG("[iterate_processes] status failed 0x%lX\n", status);
    HeapFree(GetProcessHeap(), 0, buffer);
    return;
  }

  PSYSTEM_PROCESS_INFORMATION spi = (PSYSTEM_PROCESS_INFORMATION)buffer;
  std::vector<PROCESS_INFO> temp_list;

  while (true) {
    PROCESS_INFO temp_info;

    if (spi->ImageName.Buffer) {
#pragma warning(push)
#pragma warning(disable : 4244)
      std::wstring wname = spi->ImageName.Buffer;
      temp_info.ProcessName = std::string(wname.begin(), wname.end());
#pragma warning(pop)
    }

    else {
      temp_info.ProcessName = "System";
    }

    temp_info.PID = (DWORD)(ULONG_PTR)spi->UniqueProcessId;

    DWORD count = spi->NumberOfThreads;
    for (DWORD i = 0; i < count; i++) {
      DWORD tid = (DWORD)(ULONG_PTR)spi->Threads[i].ClientId.UniqueThread;
      temp_info.TID.push_back(tid);
    }

    temp_list.push_back(temp_info);

    if (spi->NextEntryOffset == 0) break;

    spi = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)spi + spi->NextEntryOffset);
  }

  process_list = std::move(temp_list);
  HeapFree(GetProcessHeap(), 0, buffer);
}

void mem::iterate_services() {
  SC_HANDLE scm =
      OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
  if (!scm) return;

  DWORD resume_handle = 0;
  std::vector<SERVICE_INFO> temp_list;

  do {
    DWORD bytes_needed = 0;
    DWORD services_returned = 0;

    EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
                          SERVICE_STATE_ALL, nullptr, 0, &bytes_needed,
                          &services_returned, &resume_handle, nullptr);

    if (GetLastError() != ERROR_MORE_DATA || bytes_needed == 0) break;

    BYTE* buffer =
        (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes_needed);
    if (!buffer) break;

    if (!EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
                               SERVICE_STATE_ALL, buffer, bytes_needed,
                               &bytes_needed, &services_returned,
                               &resume_handle, nullptr)) {
      DBG("[iterate_services] EnumServicesStatusExA failed: 0x%lX\n",
          GetLastError());
      HeapFree(GetProcessHeap(), 0, buffer);
      break;
    }

    ENUM_SERVICE_STATUS_PROCESSA* services =
        (ENUM_SERVICE_STATUS_PROCESSA*)buffer;

    for (DWORD i = 0; i < services_returned; i++) {
      if (services[i].ServiceStatusProcess.dwProcessId == 0) continue;
      SERVICE_INFO info{};

      info.ServiceName = services[i].lpServiceName;
      info.DisplayName = services[i].lpDisplayName;
      info.PID = services[i].ServiceStatusProcess.dwProcessId;
      info.Status = services[i].ServiceStatusProcess.dwCurrentState;
      temp_list.push_back(info);
    }

    HeapFree(GetProcessHeap(), 0, buffer);

  } while (resume_handle != 0);

  service_list = std::move(temp_list);
  CloseServiceHandle(scm);
}

void mem::iterate_handles() {
  constexpr size_t hash = utils::fnv1a_compile("NtQuerySystemInformation");

  // Call the ntapi to get the ReturnLength value first but the ntapi
  // sometimes doesnt return the value. I should use this as a fallback.
  NTSTATUS status;
  ULONG ret_len = 0x10000;
  void* buffer = NULL;
  buffer = HeapAlloc(GetProcessHeap(), 0, ret_len);
  if (!buffer) return;

  status =
      SYSCALL(hash, SystemExtendedHandleInformation, buffer, ret_len, &ret_len);
  while (status == STATUS_INFO_LENGTH_MISMATCH) {
    ret_len *= 2;

    void* temp = HeapReAlloc(GetProcessHeap(), 0, buffer, ret_len);
    if (!temp) {
      HeapFree(GetProcessHeap(), 0, buffer);
      return;
    }

    buffer = temp;
    status = SYSCALL(hash, SystemExtendedHandleInformation, buffer, ret_len,
                     &ret_len);
  }

  if (!NT_SUCCESS(status)) {
    DBG("[iterate_handles] status failed 0x%lX\n", status);
    HeapFree(GetProcessHeap(), 0, buffer);
    return;
  }

  SYSTEM_HANDLE_INFORMATION_EX* shi = (SYSTEM_HANDLE_INFORMATION_EX*)buffer;
  std::vector<HANDLE_INFO> temp_list;
  for (DWORD i = 0; i < shi->NumberOfHandles; i++) {
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX handle = shi->Handles[i];

    if (handle.ObjectTypeIndex != 0x8) continue;

    HANDLE_INFO handle_info;
    handle_info.PID = (DWORD)(ULONG_PTR)handle.UniqueProcessId;
    handle_info.HandleValue = handle.HandleValue;
    handle_info.GrantedAccess = handle.GrantedAccess;
    handle_info.ObjectTypeIndex = handle.ObjectTypeIndex;

    temp_list.push_back(handle_info);
  }

  handle_list = std::move(temp_list);
  HeapFree(GetProcessHeap(), 0, buffer);
}

void mem::print_processes() {
  if (process_list.empty()) {
    iterate_processes();
  }

  for (const auto& i : process_list) {
    printf("[NAME] %s | [PID] %lu \n", i.ProcessName.c_str(), i.PID);
  }
}

void mem::print_services() {
  if (service_list.empty()) {
    iterate_services();
  }

  for (const auto& i : service_list) {
    printf("[SERVICE] %s | [DISPLAY] %s | [PID] %lu\n", i.ServiceName.c_str(),
           i.DisplayName.c_str(), i.PID);
  }
}

void mem::print_handles() {
  if (handle_list.empty()) {
    iterate_handles();
  }

  for (const auto& i : handle_list) {
    printf("[PID] %lu | [HANDLE] %p  | [ACCESS] 0x%lX\n", i.PID, i.HandleValue,
           i.GrantedAccess);
  }
}

void mem::print_threads(std::string_view process_name) {
  if (process_list.empty()) {
    iterate_processes();
  }

  for (const auto& i : process_list) {
    if (i.ProcessName == process_name) {
      for (DWORD t : i.TID) {
        printf("[TID] %lu\n", t);
      }
      return;
    }
  }
}

const std::vector<PROCESS_INFO>& mem::get_process_list() {
  return process_list;
}
const std::vector<SERVICE_INFO>& mem::get_service_list() {
  return service_list;
}
const std::vector<HANDLE_INFO>& mem::get_handle_list() { return handle_list; }
