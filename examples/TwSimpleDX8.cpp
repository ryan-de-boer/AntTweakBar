#define WIN32_LEAN_AND_MEAN 
#define POINTER_64 __ptr64
#include <windows.h>
#include <d3d8.h>

#include <tchar.h>
#include <iostream>

#include <AntTweakBar.h>

LPDIRECT3D8            pD3D = NULL;           // Direct3D interface
LPDIRECT3DDEVICE8      pD3DDevice = NULL;     // Direct3D device interface
LPDIRECT3DVERTEXBUFFER8 pVertexBuffer = NULL;  // Vertex buffer

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

  if (TwEventWin(hwnd, uMsg, wParam, lParam)) // send event message to AntTweakBar
    return 0; // event has been handled by AntTweakBar

  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int g_tweakInt = 3;

// Initialize Direct3D
bool InitD3D(HWND hWnd) {
  // Create the Direct3D interface
  pD3D = Direct3DCreate8(D3D_SDK_VERSION);
  if (!pD3D)
    return false;

  //https://royallib.com/read/Pike_Andy/DirectX_8_Programming_Tutorial.html#0
  D3DDISPLAYMODE d3ddm;
  if (FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm))) {
    return E_FAIL;
  }

  D3DPRESENT_PARAMETERS d3dpp;
  ZeroMemory(&d3dpp, sizeof(d3dpp));
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
  d3dpp.BackBufferFormat = d3ddm.Format;
  d3dpp.hDeviceWindow = hWnd;

  // Create the Direct3D device
//    HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
//        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pD3DDevice);
  HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pD3DDevice);
  if (FAILED(hr)) {
    return false;
  }

  TwInit(TW_DIRECT3D8, pD3DDevice); // for Direct3D 8
  TwDefine("SHOW_CURSOR");

  TwBar* myBar;
  myBar = TwNewBar("NameOfMyTweakBar");
  TwAddVarRW(myBar, "NameOfMyVariable", TW_TYPE_INT32, &g_tweakInt, "");

  // Set the background color to green
  pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
  return true;
}

// Main rendering loop
void Render() {
  if (!pD3DDevice)
    return;

  // Clear the screen with green color
  pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);

  // Begin the scene
  pD3DDevice->BeginScene();

  // TwWindowSize(800, 600);
  TwDraw();

  // End the scene
  pD3DDevice->EndScene();

  // Present the back buffer to the screen
  pD3DDevice->Present(NULL, NULL, NULL, NULL);
}

// Cleanup Direct3D
void Cleanup() {
  if (pD3DDevice) {
    pD3DDevice->Release();
  }
  if (pD3D) {
    pD3D->Release();
  }
}

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // Register the window class
  WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("DirectX8Window"), NULL };
  RegisterClassEx(&wc);

  // Create the window
  HWND hWnd = CreateWindow(wc.lpszClassName, _T("DirectX 8 Green Background"), WS_OVERLAPPEDWINDOW,
    100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);

  // Initialize Direct3D
  if (!InitD3D(hWnd)) {
    Cleanup();
    return 0;
  }

  // Show the window
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  // Main loop
  MSG msg;
  while (TRUE) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Render the scene
    Render();
  }

  // Cleanup and exit
  Cleanup();
  return msg.wParam;
}
