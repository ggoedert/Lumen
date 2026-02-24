//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows platform
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lEngine.h"

#include <Windows.h>

/// Lumen Windows namespace
namespace Lumen::Windows
{
    /// engine initialize config
    struct Config : Object
    {
        OBJECT_TYPEINFO;
        explicit Config(HWND window) : Object(Type()), mWindow(window) {}
        HWND mWindow;
    };

    /// start engine
    int Start(HINSTANCE hInstance, int nCmdShow, WCHAR *szTitle, WCHAR *szWindowClass, HICON hIcon, const ApplicationPtr application);

    /// create a smart pointer version of the engine
    EnginePtr CreateEngine(const ApplicationPtr &application);
}
