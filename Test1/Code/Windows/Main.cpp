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

#include "Test1.h"

#include "lEnginePlatform.h"
#include "lFramework.h"

// misc app setup
#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4061)

#ifdef USING_D3D12_AGILITY_SDK
extern "C"
{
    // Used to enable the "Agility SDK" components
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char *D3D12SDKPath = u8".\\D3D12\\";
}
#endif

#define MAX_LOADSTRING 100

WCHAR szTitle[MAX_LOADSTRING];                  // the title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/// entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(-1);  // \todo @review@ sometimes we already to late to do this, we can force _crtBreakAlloc var in watch window at dbgheap.c:392
#endif

    if (!DirectX::XMVerifyCPUSupport())
        return 1;

    Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
        return 1;

    // initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LUMENAPPLICATION, szWindowClass, MAX_LOADSTRING);

    // start engine and application
    auto engine = Lumen::Engine::MakePtr(Test1::MakePtr("Test1", 1));

    // register class and create window
    {
        // register class
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_LUMENAPPLICATION));
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm = LoadIconW(wcex.hInstance, MAKEINTRESOURCE(IDI_LUMENAPPLICATION));
        if (!RegisterClassExW(&wcex))
            return 1;

        // create window
        int width = 800;
        int height = 600;
        engine->GetDefaultSize(width, height);

        RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

#ifdef _DEBUG
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hwnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);

        if (!hwnd)
            return 1;

        ShowWindow(hwnd, nCmdShow);
#else
        AdjustWindowRect(&rc, WS_POPUP, FALSE);

        HWND hwnd = CreateWindowExW(0, szWindowClass, szTitle, WS_POPUP,
            WS_EX_TOPMOST, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);

        if (!hwnd)
            return 1;

        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
#endif

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(engine.get()));

        GetClientRect(hwnd, &rc);

        if (!engine->Initialize(Lumen::Windows::Config({ hwnd, rc.right - rc.left, rc.bottom - rc.top })))
            return 1;
    }

    // main message loop
    MSG msg = {};
    bool running = true;
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (running)
            {
                if (!engine->Tick())
                {
                    running = false;
                    PostQuitMessage(0);
                }
            }
        }
    }

    engine->Shutdown();

    return static_cast<int>(msg.wParam);
}

/// windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;
#ifdef _DEBUG
    static bool s_fullscreen = false;
#else
    static bool s_fullscreen = true;
#endif

    auto engine = reinterpret_cast<Lumen::Engine *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_PAINT:
        if (s_in_sizemove && engine)
        {
            engine->Tick();
        }
        else
        {
            PAINTSTRUCT ps;
            (void)BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DISPLAYCHANGE:
        if (engine)
        {
            engine->OnDisplayChange();
        }
        break;

    case WM_MOVE:
        if (engine)
        {
            engine->OnWindowMoved();
        }
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_in_suspend && engine)
                    engine->OnSuspending();
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_in_suspend && engine)
                engine->OnResuming();
            s_in_suspend = false;
        }
        else if (!s_in_sizemove && engine)
        {
            engine->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (engine)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            engine->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        }
        break;

    case WM_GETMINMAXINFO:
        if (lParam)
        {
            auto info = reinterpret_cast<MINMAXINFO *>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

    case WM_ACTIVATEAPP:
        if (engine)
        {
            if (wParam)
            {
                engine->OnActivated();
            }
            else
            {
                engine->OnDeactivated();
            }
        }
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && engine)
                engine->OnSuspending();
            s_in_suspend = true;
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && engine)
                    engine->OnResuming();
                s_in_suspend = false;
            }
            return TRUE;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

#ifdef _DEBUG
    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            // implements the classic ALT+ENTER fullscreen toggle
            if (s_fullscreen)
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                int width = 800;
                int height = 600;
                if (engine)
                    engine->GetDefaultSize(width, height);

                ShowWindow(hWnd, SW_SHOWNORMAL);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
            else
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                ShowWindow(hWnd, SW_SHOWMAXIMIZED);
            }

            s_fullscreen = !s_fullscreen;
        }
        break;
#endif

    case WM_MENUCHAR:
        // a menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
