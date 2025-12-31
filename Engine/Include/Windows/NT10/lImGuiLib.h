//==============================================================================================================================================================================
/// \file
/// \brief     ImGui lib for editor support
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#ifdef EDITOR
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

#define MATERIAL_ICONS_HAND   "\xee\x9d\xa4" // E764
#define MATERIAL_ICONS_MOVE   "\xee\xa2\x9f" // E89F
#define MATERIAL_ICONS_ROTATE "\xee\x80\xa8" // E028
#define MATERIAL_ICONS_SCALE  "\xef\x87\x8e" // F1CE

#define MATERIAL_ICONS_PLAY   "\xee\x80\xb7" // E037
#define MATERIAL_ICONS_PAUSE  "\xee\x80\xb4" // E034
#define MATERIAL_ICONS_STEP   "\xee\x81\x84" // E044

namespace Lumen::ImGuiLib
{
    extern ImFont *gMaterialIconsFont;
    extern const char *gMaterialIconsFontFilename;
    extern ImWchar gMaterialIconsIconsRanges[];
    extern float gMaterialIconsFontSize;
    extern float gMaterialIconsIconsGlyphOffset;

    /// custom version ImGui::Button with round value and draw flags
    bool Button(const char *label, float round_val, ImDrawFlags draw_flags = ImDrawFlags_None, const ImVec2 &size = ImVec2(0, 0));
}
#endif
