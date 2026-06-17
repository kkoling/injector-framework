#include "vars.h"

namespace vars {
// syscall
bool syscall_indirect = false;
bool proxy_call = true;

// inject
std::pair<std::string, DWORD> process_selected{};
bool dupe_handle = false;
bool hijack_handle = false;
bool hijack_thread = false;
int injection_type = LOAD_LIBRARY;
}  // namespace vars