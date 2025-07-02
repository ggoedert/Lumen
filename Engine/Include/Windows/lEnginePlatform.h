//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows platform
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

/// Lumen Windows namespace
namespace Lumen::Windows
{
    /// engine initialize config
    struct Config : Object
    {
        OBJECT_TRAITS;
        explicit Config(HWND window, int width, int height) : Object(Type()), window(window), width(width), height(height) {}
        HWND window; int width; int height;
    };
}
