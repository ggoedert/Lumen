//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows platform
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lObject.h"

#include <Windows.h>

/// Lumen Windows namespace
namespace Lumen::Windows
{
    /// engine initialize config
    struct Config : Object
    {
        OBJECT_TYPEINFO;
        explicit Config(HWND window, int width, int height) : Object(Type()), mWindow(window), mWidth(width), mHeight(height) {}
        HWND mWindow; int mWidth; int mHeight;
    };
}
