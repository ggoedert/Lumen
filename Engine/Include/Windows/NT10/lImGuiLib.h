//==============================================================================================================================================================================
/// \file
/// \brief     ImGui lib for editor support
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#ifdef EDITOR
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"
#endif
