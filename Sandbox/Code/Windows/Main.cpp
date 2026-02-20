//==============================================================================================================================================================================
/// \file
/// \brief     main windows entry point
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

// windows debug
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "resource.h"
#include "SimpleMath.h"

#include "Sandbox.h"

#include "lEngineWindows.h"
#include "lFramework.h"

// misc app setup
#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#if (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)
#ifdef USING_D3D12_AGILITY_SDK
extern "C"
{
    // Used to enable the "Agility SDK" components
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char *D3D12SDKPath = u8".\\D3D12\\";
}
#endif
#else
// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#define MAX_LOADSTRING 100

WCHAR szTitle[MAX_LOADSTRING];                  // the title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

/// entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(-1);  // \todo @review@ sometimes we already to late to do this, we can force _crtBreakAlloc var in watch window at dbgheap.c:392
#endif

    // tells Windows not to virtualize/stretch window coordinates
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (!DirectX::XMVerifyCPUSupport())
        return 1;

#if (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)
    Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
        return 1;
#else
    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;
#endif

    // initialize data
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LUMENAPPLICATION, szWindowClass, MAX_LOADSTRING);
    HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_LUMENAPPLICATION));

    // start engine and application
    auto application = Sandbox::MakePtr("Sandbox", 1);
    int result = Lumen::Windows::Start(hInstance, nCmdShow, szTitle, szWindowClass, hIcon, application);

#if (_WIN32_WINNT < _WIN32_WINNT_WINTHRESHOLD)
    CoUninitialize();
#endif

    return result;
}
