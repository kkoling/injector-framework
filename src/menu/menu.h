#pragma once
#include <d3d11.h>

#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_impl_dx11.h"
#include "../../external/imgui/imgui_impl_win32.h"
#include "../../external/imgui/imgui_stdlib.h"
#include "../core/inj.h"

typedef struct {
  HWND Hwnd;
  ID3D11Device* D3Device;
  ID3D11DeviceContext* D3DeviceContext;
  IDXGISwapChain* D3SwapChain;
  ID3D11RenderTargetView* RenderTargetView;
  int WindowWidth;
  int WindowHeight;
} D3D_CONTEXT, *PD3D_CONTEXT;

namespace menu {
void render();

void draw();
void style_winxp();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}  // namespace menu
