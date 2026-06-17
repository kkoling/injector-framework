#include "core/inj.h"
#include "menu/menu.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  if (!utils::enable_admin_privilege() || !utils::enable_debug_privilege()) {
    MessageBox(NULL, "Failed to get admin and debug privilege", "Error",
               MB_OK | MB_ICONERROR);
    return 0;
  }

#ifdef NDEBUG
#else
  if (!AllocConsole()) {
    MessageBox(NULL, "Failed to alloc console", "Error", MB_OK | MB_ICONERROR);
    return 0;
  }

  SetConsoleTitle("winbs-debug");
  FILE* fp;
  freopen_s(&fp, "CONIN$", "r", stdin);
  freopen_s(&fp, "CONOUT$", "w", stdout);
  freopen_s(&fp, "CONOUT$", "w", stderr);

#endif

  if (!sys::init()) {
    MessageBox(NULL, "Failed to initialize syscall", "Error",
               MB_OK | MB_ICONERROR);
    FreeConsole();
    return 0;
  }

  menu::render();

#ifdef NDEBUG
#else
  FreeConsole();
#endif
  return 0;
}
