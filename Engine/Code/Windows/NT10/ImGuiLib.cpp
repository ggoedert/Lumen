//==============================================================================================================================================================================
/// \file
/// \brief     ImGui lib for editor support
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifdef EDITOR
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_tables.cpp"
#ifdef SHOW_DEMO_WINDOW
#include "imgui_demo.cpp"
#endif
#include "backends/imgui_impl_win32.cpp"
#include "backends/imgui_impl_dx12.cpp"
#endif
