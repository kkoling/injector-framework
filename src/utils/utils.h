#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include "winapi.h"

// i tried to keep this as primitive as possible. no fancy c++ features, just
// raw winapi calls and basic c++ syntax. this is to avoid any unnecessary
// dependencies or complications.

constexpr size_t FNV1A_64_OFFSET = 0xcbf29ce484222325ULL;
constexpr size_t FNV1A_64_PRIME = 0x100000001b3ULL;

#ifdef NDEBUG
#define DBG(...)
#else
#define DBG(...) printf(__VA_ARGS__)
#endif

namespace utils {
void* get_ntdll();
void* get_module_address(const char* mod_name);
void* get_module_address_w(const wchar_t* mod_name);
void* get_module_address_hash(size_t hash);  // use fnv1w for hashing
void* get_export_address(void* module_addr, const char* export_name);
void* get_export_address_hash(void* module_addr, size_t export_name_hash);

bool enable_admin_privilege();
bool enable_debug_privilege();

void chartowchar(char* in_str, wchar_t* out_str, size_t out_len);
void wchartochar(wchar_t* in_str, char* out_str, size_t out_len);

void rtl_init_unicode_string(UNICODE_STRING* unicode_string,
                             const wchar_t* wstring);
void rtl_empty_unicode_string(UNICODE_STRING* unicode_string);

size_t fnv1a(const char* string);
size_t fnv1w(const wchar_t* wstring);
constexpr size_t fnv1a_compile(const char* string) {
  size_t hash = FNV1A_64_OFFSET;
  while (*string) {
    hash ^= static_cast<unsigned char>(*string++);
    hash *= FNV1A_64_PRIME;
  }
  return hash;
}
constexpr size_t fnv1w_compile(const wchar_t* wstring) {
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

// ai
bool name_matches(const std::string& name, const std::string& filter);
}  // namespace utils