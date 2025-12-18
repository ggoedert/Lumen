//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindows.h"

/// Lumen Windows namespace
namespace Lumen::Windows
{
    extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
}
