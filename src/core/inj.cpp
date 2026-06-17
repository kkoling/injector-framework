#include "inj.h"

#define MSG_ERR(text) MessageBoxA(NULL, text, "Error", MB_OK | MB_ICONERROR);
#define MSG_SUCC(text) \
  MessageBoxA(NULL, text, "Success", MB_OK | MB_ICONEXCLAMATION);

static std::vector<DLL_INFO> dll_list;

bool inj::inject() {
  if (dll_list.empty()) {
    MSG_ERR("Dll not loaded");
    return false;
  }

  if (vars::process_selected.first.empty() ||
      vars::process_selected.second == 0) {
    MSG_ERR("Process not selected\n");
    return false;
  }

  HANDLE proc_handle = mem::open_proc_handle(vars::process_selected.second);
  if (!proc_handle) {
    MSG_ERR("Failed to open process handle");
    return false;
  }

  bool success = false;
  for (const auto& i : inj::get_dll_list()) {
    switch (vars::injection_type) {
      case LOAD_LIBRARY: {
        success = load_library_inj(proc_handle, i);
        if (!success) {
          MSG_ERR("Failed LoadLibrary Injection");
        } else {
          MSG_SUCC("LoadLibrary Injection Success");
        }
      } break;
      case MANUAL_MAP: {
      } break;
      case MANAUAL_MAP_W_SHELLCODE: {
      } break;
    }
  }

  return success;
}

bool inj::load_library_inj(HANDLE proc_handle, const DLL_INFO& dll_info) {
  void* remote_buffer =
      NtAllocMem(proc_handle, dll_info.FilePath.length() + 1,
                 MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!remote_buffer) return false;

  bool success = false;

  if (NtWriteMem(proc_handle, remote_buffer, (PVOID)dll_info.FilePath.data(),
                 dll_info.FilePath.length() + 1, NULL)) {
    if (vars::hijack_thread) {
    } else {
      if (mem::create_remote_thread(proc_handle, (void*)LoadLibraryA,
                                    remote_buffer)) {
        DBG("[load_library_inject] load lib success\n");
        success = true;
      }
    }
  }

  if (!success && remote_buffer != nullptr)
    NtFreeMem(proc_handle, remote_buffer);

  return success;
}

bool inj::open_file_dialog() {
  OPENFILENAMEA ofn = {};
  char path[MAX_PATH * 32] = {};
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = path;
  ofn.nMaxFile = sizeof(path);
  ofn.lpstrFilter = "DLL Files (*.dll)\0*.dll\0";
  ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

  if (GetOpenFileNameA(&ofn)) {
    char* dir = ofn.lpstrFile;
    char* file = dir + strlen(dir) + 1;

    // single file
    if (*file == '\0') {
      return load_dll_from_disk(path);
    }

    // multiple file
    bool is_done = false;
    while (*file != '\0') {
      std::string multi_path = std::string(dir) + "\\" + file;
      is_done = load_dll_from_disk(multi_path);
      if (!is_done) DBG("failed to open %s\n", multi_path.c_str());
      file += strlen(file) + 1;
    }

    return true;
  }

  return false;
}

// i dont add a check for if the file is a dll. i put my trust on
// GetOpenFileNameA function. Open the file and parse through
// IMAGE_NT_HEADERS->IMAGE_FILE_HEADER->Characteristics & IMAGE_FILE_DLL
bool inj::load_dll_from_disk(std::string file_path) {
  if (file_path.empty()) {
    DBG("[load_dll_from_disk] file path is empty\n");
    return false;
  }

  std::wstring wpath(file_path.begin(), file_path.end());

  UNICODE_STRING nt_path{};
  constexpr size_t hash_dos =
      utils::fnv1a_compile("RtlDosPathNameToRelativeNtPathName_U");
  auto fn_dos =
      (RtlDosPathNameToRelativeNtPathName_U)(utils::get_export_address_hash(
          utils::get_ntdll(), hash_dos));

  if (!fn_dos || !fn_dos(wpath.c_str(), &nt_path, nullptr, nullptr)) {
    DBG("[load_dll_from_disk] RtlDosPathNameToRelativeNtPathName_U failed\n");
    return false;
  }

  OBJECT_ATTRIBUTES obj{};
  IO_STATUS_BLOCK io{};
  InitializeObjectAttributes(&obj, &nt_path, OBJ_CASE_INSENSITIVE, nullptr,
                             nullptr);

  constexpr size_t hash_create = utils::fnv1a_compile("NtCreateFile");

  HANDLE file_ptr = nullptr;
  NTSTATUS status =
      SYSCALL(hash_create, &file_ptr, FILE_GENERIC_READ, &obj, &io, nullptr,
              FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
              FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, nullptr, 0);

  utils::rtl_empty_unicode_string(&nt_path);

  if (!NT_SUCCESS(status) || !file_ptr) {
    DBG("[load_dll_from_disk] NtCreateFile failed: 0x%lX\n", status);
    return false;
  }

  constexpr size_t hash_query = utils::fnv1a_compile("NtQueryInformationFile");

  FILE_STANDARD_INFORMATION file_info{};
  status = SYSCALL(hash_query, file_ptr, &io, &file_info, sizeof(file_info),
                   FileStandardInformation);
  if (!NT_SUCCESS(status) || file_info.EndOfFile.QuadPart == 0) {
    DBG("[load_dll_from_disk] NtQueryInformationFile failed: 0x%lX\n", status);
    NtCloseHandle(file_ptr);
    return false;
  }

  DWORD file_size = (DWORD)(ULONG_PTR)file_info.EndOfFile.QuadPart;

  void* file_buffer = NtAllocMem(NtCurrentProcess(), file_size,
                                 MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!file_buffer) {
    DBG("[load_dll_from_disk] alloc mem failed\n");
    NtCloseHandle(file_ptr);
    return false;
  }

  constexpr size_t hash_read = utils::fnv1a_compile("NtReadFile");

  status = SYSCALL(hash_read, file_ptr, nullptr, nullptr, nullptr, &io,
                   file_buffer, file_size, nullptr, nullptr);
  NtCloseHandle(file_ptr);

  if (!NT_SUCCESS(status)) {
    DBG("[load_dll_from_disk] NtReadFile failed: 0x%lX\n", status);
    NtFreeMem(NtCurrentProcess(), file_buffer);
    return false;
  }

  // DBG("[load_dll_from_disk] loaded %lu bytes at %p\n", file_size,
  // file_buffer);
  std::string file_name = file_path.substr(file_path.find_last_of("\\/") + 1);

  DLL_INFO temp_info;
  temp_info.FileName = file_name;
  temp_info.FilePath = file_path;
  temp_info.FileBuffer = file_buffer;
  temp_info.FileSize = file_size;

  dll_list.push_back(temp_info);

  return true;
}

const std::vector<DLL_INFO>& inj::get_dll_list() { return dll_list; }

void inj::empty_dll_list() { dll_list.clear(); }