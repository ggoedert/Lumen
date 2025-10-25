//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindows.h"

// helpers headers
#include "EngineFramework.h"

/// Lumen Windows namespace
namespace Lumen::Windows
{
    LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    /// start engine
    int Start(HINSTANCE hInstance, int nCmdShow, WCHAR *szTitle, WCHAR *szWindowClass, HICON hIcon, EnginePtr engine)
    {
        // register class and create window
        {
            // register class
            WNDCLASSEXW wcex = {};
            wcex.cbSize = sizeof(WNDCLASSEXW);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = WndProc;
            wcex.hInstance = hInstance;
            wcex.hIcon = hIcon;
            wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            wcex.lpszClassName = szWindowClass;
            wcex.hIconSm = hIcon;
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

            if (!engine->Initialize(Lumen::Windows::Config(hwnd, rc.right - rc.left, rc.bottom - rc.top)))
                return 1;

            if (!engine->Open())
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
                    if (!engine->Run())
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
                engine->Run();
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
}
