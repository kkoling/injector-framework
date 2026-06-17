#pragma once
#include "mem.h"

typedef struct {
  std::string FileName;
  std::string FilePath;
  void* FileBuffer;
  DWORD FileSize;
} DLL_INFO;

namespace inj {
bool inject();

bool load_library_inj(HANDLE proc_handle, const DLL_INFO& dll_info);
bool manual_map_inj(HANDLE proc_handle, const DLL_INFO& dll_info);

bool module_base_reloc();
bool module_imports_reloc();

bool open_file_dialog();
bool load_dll_from_disk(std::string file_path);

const std::vector<DLL_INFO>& get_dll_list();
void empty_dll_list();
}  // namespace inj

static unsigned char dll_stub[] = {
    0x48, 0x83, 0xEC,
    0x38,  // sub rsp,0x38 (shadow space + alignment + 8 bytes)
           // RCX = hModule (1st param)
    0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,  // mov rcx,[hModule]
           // EDX = fdwReason (DLL_PROCESS_ATTACH = 1, 2nd param)
    0xBA, 0x01, 0x00, 0x00, 0x00,  // mov edx,1
                                   // R8 = lpvReserved (NULL, 3rd param)
    0x49, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,  // mov r8,0
           // DllMain
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,                    // mov rax,[DllMain]
    0xFF, 0xD0,              // call rax
    0x48, 0x83, 0xC4, 0x38,  // add rsp,0x38
    0xC3                     // ret
};