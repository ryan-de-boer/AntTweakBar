#include <windows.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <AntTweakBar.h>
#include <iostream>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "AntTweakBar64.lib")

const UINT kFrameCount = 2;

// Globals
ID3D12Device* g_Device = nullptr;
ID3D12CommandQueue* g_CommandQueue = nullptr;
IDXGISwapChain3* g_SwapChain = nullptr;
ID3D12DescriptorHeap* g_RTVHeap = nullptr;
ID3D12Resource* g_RenderTargets[kFrameCount];
ID3D12CommandAllocator* g_CommandAllocator = nullptr;
ID3D12GraphicsCommandList* g_CommandList = nullptr;
ID3D12Fence* g_Fence = nullptr;
HANDLE g_FenceEvent;
UINT64 g_FenceValue = 0;
UINT g_FrameIndex = 0;

void InitD3D(HWND hwnd);
void Cleanup();
void WaitForPreviousFrame();
void Render();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int g_tweak = 2;

int g_clientWidth=800;
int g_clientHeight=600;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Register the window class
  const wchar_t CLASS_NAME[] = L"Sample Window Class";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClass(&wc);

  RECT clientRect = { 0, 0, 800, 600 };
  AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE); // Adjust for non-client area

  int windowWidth = clientRect.right - clientRect.left;
  int windowHeight = clientRect.bottom - clientRect.top;

  // Create the window
  HWND hwnd = CreateWindowEx(
    0,
    CLASS_NAME,
    L"DirectX 12 Hello World",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
    nullptr,
    nullptr,
    hInstance,
    nullptr
  );

  if (hwnd == nullptr)
  {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  // Initialize Direct3D
  InitD3D(hwnd);

  // Initialize AntTweakBar
  TwInit(TW_DIRECT3D12, g_Device);
  TwDefine("SHOW_CURSOR");

  TwBar* myBar;
  myBar = TwNewBar("NameOfMyTweakBar");

  TwAddVarRW(myBar, "NameOfMyVariable", TW_TYPE_INT32, &g_tweak, "");

  TwAddVarRO(myBar, "g_clientWidth", TW_TYPE_INT32, &g_clientWidth, "");
  TwAddVarRO(myBar, "g_clientHeight", TW_TYPE_INT32, &g_clientHeight, "");


  // Main loop
  MSG msg = {};
  while (msg.message != WM_QUIT)
  {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      Render();
    }
  }

  // Cleanup
  Cleanup();
  TwTerminate();

  return 0;
}

void InitD3D(HWND hwnd)
{
  // Create device
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_Device));

  // Create command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  g_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_CommandQueue));

  // Create swap chain
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = kFrameCount;
  swapChainDesc.Width = 800;
  swapChainDesc.Height = 600;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;

  IDXGIFactory4* factory;
  CreateDXGIFactory1(IID_PPV_ARGS(&factory));
  factory->CreateSwapChainForHwnd(g_CommandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&g_SwapChain));

  // Create descriptor heap for render target views
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = kFrameCount;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  g_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_RTVHeap));

  // Create render target views
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_RTVHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < kFrameCount; i++)
  {
    g_SwapChain->GetBuffer(i, IID_PPV_ARGS(&g_RenderTargets[i]));
    g_Device->CreateRenderTargetView(g_RenderTargets[i], nullptr, rtvHandle);
    rtvHandle.Offset(1, g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
  }

  // Create command allocator and command list
  g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_CommandAllocator));
  g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_CommandAllocator, nullptr, IID_PPV_ARGS(&g_CommandList));

  // Command lists are created in the recording state, so close it before using it
  g_CommandList->Close();

  // Create synchronization objects
  g_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_Fence));
  g_FenceValue = 1;
  g_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void Cleanup()
{
  WaitForPreviousFrame();

  CloseHandle(g_FenceEvent);

  for (UINT i = 0; i < kFrameCount; i++)
  {
    g_RenderTargets[i]->Release();
  }

  g_CommandList->Release();
  g_CommandAllocator->Release();
  g_RTVHeap->Release();
  g_SwapChain->Release();
  g_CommandQueue->Release();
  g_Device->Release();
}

void WaitForPreviousFrame()
{
  const UINT64 fence = g_FenceValue;
  g_CommandQueue->Signal(g_Fence, fence);
  g_FenceValue++;

  if (g_Fence->GetCompletedValue() < fence)
  {
    g_Fence->SetEventOnCompletion(fence, g_FenceEvent);
    WaitForSingleObject(g_FenceEvent, INFINITE);
  }

  g_FrameIndex = g_SwapChain->GetCurrentBackBufferIndex();
}

void Render()
{
  // Record commands
  g_CommandAllocator->Reset();
  g_CommandList->Reset(g_CommandAllocator, nullptr);

  // Set the render target
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_RTVHeap->GetCPUDescriptorHandleForHeapStart(), g_FrameIndex, g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
  g_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

  // Clear the render target
  const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // White color
  g_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

  // Draw AntTweakBar UI
  TwWindowSize(g_clientWidth, g_clientHeight);
  TwDrawContext(g_CommandList);

  // Indicate that the back buffer will now be used to present
  g_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_RenderTargets[g_FrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

  // Close the command list
  g_CommandList->Close();

  // Execute the command list
  ID3D12CommandList* ppCommandLists[] = { g_CommandList };
  g_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // Present the frame
  g_SwapChain->Present(1, 0);

  WaitForPreviousFrame();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  RECT rect;
  if (GetClientRect(hwnd, &rect))
  {
    g_clientWidth = rect.right - rect.left;
    g_clientHeight = rect.bottom - rect.top;
  }

  if (TwEventWin(hwnd, uMsg, wParam, lParam))
    return 0;

  switch (uMsg)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}