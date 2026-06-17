#include "menu.h"

static D3D_CONTEXT d3d{};
static WNDCLASSEXA wc{};
static bool is_running = true;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);

void menu::style_winxp() {
  ImGuiStyle& style = ImGui::GetStyle();

  // Sharp corners — XP had no rounding
  style.WindowRounding = 0.0f;
  style.FrameRounding = 0.0f;
  style.ScrollbarRounding = 0.0f;
  style.GrabRounding = 0.0f;
  style.TabRounding = 0.0f;

  // Sizing closer to classic Windows
  style.FramePadding = ImVec2(4, 3);
  style.ItemSpacing = ImVec2(8, 4);
  style.ScrollbarSize = 16.0f;
  style.GrabMinSize = 16.0f;

  ImVec4* c = style.Colors;

  // XP Luna Blue palette
  c[ImGuiCol_WindowBg] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);  // gray dialog bg
  c[ImGuiCol_ChildBg] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
  c[ImGuiCol_PopupBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);

  c[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.13f, 0.80f, 1.00f);  // XP blue title
  c[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.27f, 0.98f, 1.00f);
  c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.13f, 0.80f, 0.75f);

  c[ImGuiCol_MenuBarBg] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);

  c[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // white input bg
  c[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.94f, 1.00f, 1.00f);
  c[ImGuiCol_FrameBgActive] = ImVec4(0.80f, 0.88f, 1.00f, 1.00f);

  c[ImGuiCol_Button] =
      ImVec4(0.88f, 0.88f, 0.88f, 1.00f);  // classic button gray
  c[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.88f, 1.00f, 1.00f);
  c[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.27f, 0.98f, 1.00f);

  c[ImGuiCol_Header] =
      ImVec4(0.06f, 0.27f, 0.98f, 1.00f);  // selected item blue
  c[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.47f, 0.98f, 1.00f);
  c[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.27f, 0.98f, 1.00f);

  c[ImGuiCol_SliderGrab] = ImVec4(0.06f, 0.27f, 0.98f, 1.00f);
  c[ImGuiCol_SliderGrabActive] = ImVec4(0.04f, 0.18f, 0.78f, 1.00f);

  c[ImGuiCol_CheckMark] = ImVec4(0.06f, 0.27f, 0.98f, 1.00f);

  c[ImGuiCol_ScrollbarBg] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
  c[ImGuiCol_ScrollbarGrab] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

  c[ImGuiCol_Separator] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  c[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

  c[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);  // black text
  c[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
}

LRESULT CALLBACK menu::WndProc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;

  switch (msg) {
    case WM_SIZE:
      if (d3d.D3Device && wParam != SIZE_MINIMIZED) {
        if (d3d.RenderTargetView) {
          d3d.RenderTargetView->Release();
          d3d.RenderTargetView = NULL;
        }

        d3d.D3SwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam),
                                       (UINT)HIWORD(lParam),
                                       DXGI_FORMAT_UNKNOWN, 0);

        ID3D11Texture2D* back_buffer = nullptr;
        if (SUCCEEDED(
                d3d.D3SwapChain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)))) {
          d3d.D3Device->CreateRenderTargetView(back_buffer, nullptr,
                                               &d3d.RenderTargetView);
          back_buffer->Release();
        }
      }
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void menu::render() {
  ImGui_ImplWin32_EnableDpiAwareness();
  d3d.WindowHeight = 400;
  d3d.WindowWidth = 350;
  // init window

  wc.cbSize = sizeof(WNDCLASSEXA);
  wc.style = CS_CLASSDC;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandleA(NULL);
  wc.lpszClassName = "Injector Menu";

  RegisterClassExA(&wc);
  d3d.Hwnd =
      CreateWindowA(wc.lpszClassName, "Windows Injector Framework",
                    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                    CW_USEDEFAULT, CW_USEDEFAULT, d3d.WindowWidth,
                    d3d.WindowHeight, NULL, NULL, wc.hInstance, NULL);

  if (!d3d.Hwnd) {
    UnregisterClassA(wc.lpszClassName, wc.hInstance);
    return;
  }

  // d3d device and swap chain
  D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };

  DXGI_SWAP_CHAIN_DESC sd{};
  sd.BufferCount = 2;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = d3d.Hwnd;
  sd.SampleDesc.Count = 1;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  HRESULT result = D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, feature_levels,
      ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &sd, &d3d.D3SwapChain,
      &d3d.D3Device, NULL, &d3d.D3DeviceContext);

  if (FAILED(result)) {
    DestroyWindow(d3d.Hwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);
    return;
  }

  // create render target
  ID3D11Texture2D* back_buffer = nullptr;
  if (SUCCEEDED(d3d.D3SwapChain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)))) {
    d3d.D3Device->CreateRenderTargetView(back_buffer, nullptr,
                                         &d3d.RenderTargetView);
    back_buffer->Release();
  }

  ShowWindow(d3d.Hwnd, SW_SHOWDEFAULT);
  UpdateWindow(d3d.Hwnd);

  // imgui stuff
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;

  // style_winxp();

  ImFont* font =
      io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibri.ttf", 14);
  if (!font) font = io.Fonts->AddFontDefault();

  ImGui_ImplWin32_Init(d3d.Hwnd);
  ImGui_ImplDX11_Init(d3d.D3Device, d3d.D3DeviceContext);

  // render loop
  const float clear_color[4] = {0.1f, 0.1f, 0.1f, 1.0f};
  MSG msg{};
  while (is_running) {
    while (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);

      if (msg.message == WM_QUIT) is_running = false;
    }

    if (!is_running) break;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(font);
    draw();
    ImGui::PopFont();
    ImGui::Render();

    d3d.D3DeviceContext->OMSetRenderTargets(1, &d3d.RenderTargetView, NULL);
    d3d.D3DeviceContext->ClearRenderTargetView(d3d.RenderTargetView,
                                               clear_color);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    d3d.D3SwapChain->Present(1, 0);
  }

  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  if (d3d.RenderTargetView) {
    d3d.RenderTargetView->Release();
    d3d.RenderTargetView = nullptr;
  }
  if (d3d.D3SwapChain) {
    d3d.D3SwapChain->Release();
    d3d.D3SwapChain = nullptr;
  }
  if (d3d.D3DeviceContext) {
    d3d.D3DeviceContext->Release();
    d3d.D3DeviceContext = nullptr;
  }
  if (d3d.D3Device) {
    d3d.D3Device->Release();
    d3d.D3Device = nullptr;
  }

  DestroyWindow(d3d.Hwnd);
  UnregisterClassA(wc.lpszClassName, wc.hInstance);
}