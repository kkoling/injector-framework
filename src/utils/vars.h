#pragma once
#include "utils.h"

typedef enum {
  LOAD_LIBRARY = 0,
  MANUAL_MAP,
  MANAUAL_MAP_W_SHELLCODE
} INJECTION_FLAGS;

namespace vars {
// syscall
extern bool syscall_indirect;
extern bool proxy_call;

// inject
extern std::pair<std::string, DWORD> process_selected;
extern bool dupe_handle;
extern bool hijack_handle;
extern bool hijack_thread;
extern int injection_type;
}  // namespace vars