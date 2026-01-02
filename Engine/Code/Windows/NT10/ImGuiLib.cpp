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
#include "backends/imgui_impl_win32.cpp"
#include "backends/imgui_impl_dx12.cpp"

#include "lDebugLog.h"

/// Lumen ImGuiLib namespace
namespace Lumen::ImGuiLib
{
    /// material icons font setup
    ImFont *gMaterialIconsFont = nullptr;
    const char *gMaterialIconsFontFilename = "Data\\material-design-icons\\font\\MaterialIcons-Regular.ttf";
    ImWchar gMaterialIconsIconsRanges[] = { 0xE000, 0xF8FF, 0 };
    float gMaterialIconsFontSize = 16.f;
    float gMaterialIconsIconsGlyphOffset = (gMaterialIconsFontSize-6.f)*0.5f;
    ImVec4 gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Count)];

    /// lumen theme
    void LumenTheme(bool nLight)
    {
        ImGuiStyle *style = &ImGui::GetStyle();
        ImVec4 *colors = style->Colors;

        if (nLight)
        {
            colors[ImGuiCol_Text]                      = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TextDisabled]              = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_WindowBg]                  = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
            colors[ImGuiCol_ChildBg]                   = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg]                   = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
            colors[ImGuiCol_Border]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
            colors[ImGuiCol_BorderShadow]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_FrameBgHovered]            = ImVec4(0.55f, 0.55f, 0.55f, 0.40f);
            colors[ImGuiCol_FrameBgActive]             = ImVec4(0.55f, 0.55f, 0.55f, 0.67f);
            colors[ImGuiCol_TitleBg]                   = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
            colors[ImGuiCol_TitleBgActive]             = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]          = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
            colors[ImGuiCol_MenuBarBg]                 = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
            colors[ImGuiCol_ScrollbarBg]               = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
            colors[ImGuiCol_ScrollbarGrab]             = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
            colors[ImGuiCol_ScrollbarGrabHovered]      = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
            colors[ImGuiCol_ScrollbarGrabActive]       = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
            colors[ImGuiCol_CheckMark]                 = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_SliderGrab]                = ImVec4(0.55f, 0.55f, 0.55f, 0.78f);
            colors[ImGuiCol_SliderGrabActive]          = ImVec4(0.54f, 0.54f, 0.54f, 0.60f);
            colors[ImGuiCol_Button]                    = ImVec4(0.55f, 0.55f, 0.55f, 0.40f);
            colors[ImGuiCol_ButtonHovered]             = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_ButtonActive]              = ImVec4(0.46f, 0.46f, 0.46f, 1.00f);
            colors[ImGuiCol_Header]                    = ImVec4(0.55f, 0.55f, 0.55f, 0.31f);
            colors[ImGuiCol_HeaderHovered]             = ImVec4(0.55f, 0.55f, 0.55f, 0.80f);
            colors[ImGuiCol_HeaderActive]              = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_Separator]                 = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
            colors[ImGuiCol_SeparatorHovered]          = ImVec4(0.40f, 0.40f, 0.40f, 0.78f);
            colors[ImGuiCol_SeparatorActive]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
            colors[ImGuiCol_ResizeGrip]                = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
            colors[ImGuiCol_ResizeGripHovered]         = ImVec4(0.55f, 0.55f, 0.55f, 0.67f);
            colors[ImGuiCol_ResizeGripActive]          = ImVec4(0.55f, 0.55f, 0.55f, 0.95f);
            colors[ImGuiCol_InputTextCursor]           = colors[ImGuiCol_Text];
            colors[ImGuiCol_TabHovered]                = colors[ImGuiCol_HeaderHovered];
            colors[ImGuiCol_Tab]                       = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.90f);
            colors[ImGuiCol_TabSelected]               = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
            colors[ImGuiCol_TabSelectedOverline]       = colors[ImGuiCol_HeaderActive];
            colors[ImGuiCol_TabDimmed]                 = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
            colors[ImGuiCol_TabDimmedSelected]         = ImLerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
            colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.55f, 0.55f, 0.55f, 0.00f);
            colors[ImGuiCol_DockingPreview]            = colors[ImGuiCol_Header] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
            colors[ImGuiCol_DockingEmptyBg]            = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_PlotLines]                 = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered]          = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_PlotHistogram]             = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered]      = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
            colors[ImGuiCol_TableHeaderBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
            colors[ImGuiCol_TableBorderStrong]         = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);   // Prefer using Alpha=1.0 here
            colors[ImGuiCol_TableBorderLight]          = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);   // Prefer using Alpha=1.0 here
            colors[ImGuiCol_TableRowBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_TableRowBgAlt]             = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
            colors[ImGuiCol_TextLink]                  = colors[ImGuiCol_HeaderActive];
            colors[ImGuiCol_TextSelectedBg]            = ImVec4(0.55f, 0.55f, 0.55f, 0.35f);
            colors[ImGuiCol_TreeLines]                 = colors[ImGuiCol_Border];
            colors[ImGuiCol_DragDropTarget]            = ImVec4(0.55f, 0.55f, 0.55f, 0.95f);
            colors[ImGuiCol_UnsavedMarker]             = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_NavCursor]                 = colors[ImGuiCol_HeaderHovered];
            colors[ImGuiCol_NavWindowingHighlight]     = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg]         = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg]          = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Detail)]  = { 0.667f, 0.667f, 0.667f, 1.f };
            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Info)]    = { 0.500f, 0.500f, 0.500f, 1.f };
            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Warning)] = { 0.851f, 0.467f, 0.024f, 1.f };
            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Error)]   = { 1.000f, 0.333f, 0.333f, 1.f };
        }
        else
        {
            colors[ImGuiCol_Text]                      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_TextDisabled]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            colors[ImGuiCol_WindowBg]                  = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
            colors[ImGuiCol_ChildBg]                   = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg]                   = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
            colors[ImGuiCol_Border]                    = ImVec4(0.44f, 0.44f, 0.44f, 0.50f);
            colors[ImGuiCol_BorderShadow]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg]                   = ImVec4(0.28f, 0.28f, 0.28f, 0.54f);
            colors[ImGuiCol_FrameBgHovered]            = ImVec4(0.55f, 0.55f, 0.55f, 0.40f);
            colors[ImGuiCol_FrameBgActive]             = ImVec4(0.55f, 0.55f, 0.55f, 0.67f);
            colors[ImGuiCol_TitleBg]                   = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
            colors[ImGuiCol_TitleBgActive]             = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]          = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
            colors[ImGuiCol_MenuBarBg]                 = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_ScrollbarBg]               = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
            colors[ImGuiCol_ScrollbarGrab]             = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]      = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]       = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
            colors[ImGuiCol_CheckMark]                 = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_SliderGrab]                = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
            colors[ImGuiCol_SliderGrabActive]          = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_Button]                    = ImVec4(0.55f, 0.55f, 0.55f, 0.40f);
            colors[ImGuiCol_ButtonHovered]             = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_ButtonActive]              = ImVec4(0.46f, 0.46f, 0.46f, 1.00f);
            colors[ImGuiCol_Header]                    = ImVec4(0.55f, 0.55f, 0.55f, 0.31f);
            colors[ImGuiCol_HeaderHovered]             = ImVec4(0.55f, 0.55f, 0.55f, 0.80f);
            colors[ImGuiCol_HeaderActive]              = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_Separator]                 = colors[ImGuiCol_Border];
            colors[ImGuiCol_SeparatorHovered]          = ImVec4(0.36f, 0.36f, 0.36f, 0.78f);
            colors[ImGuiCol_SeparatorActive]           = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
            colors[ImGuiCol_ResizeGrip]                = ImVec4(0.55f, 0.55f, 0.55f, 0.20f);
            colors[ImGuiCol_ResizeGripHovered]         = ImVec4(0.55f, 0.55f, 0.55f, 0.67f);
            colors[ImGuiCol_ResizeGripActive]          = ImVec4(0.55f, 0.55f, 0.55f, 0.95f);
            colors[ImGuiCol_InputTextCursor]           = colors[ImGuiCol_Text];
            colors[ImGuiCol_TabHovered]                = colors[ImGuiCol_HeaderHovered];
            colors[ImGuiCol_Tab]                       = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
            colors[ImGuiCol_TabSelected]               = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
            colors[ImGuiCol_TabSelectedOverline]       = colors[ImGuiCol_HeaderActive];
            colors[ImGuiCol_TabDimmed]                 = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
            colors[ImGuiCol_TabDimmedSelected]         = ImLerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
            colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
            colors[ImGuiCol_DockingPreview]            = colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
            colors[ImGuiCol_DockingEmptyBg]            = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_PlotLines]                 = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered]          = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_PlotHistogram]             = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered]      = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
            colors[ImGuiCol_TableHeaderBg]             = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
            colors[ImGuiCol_TableBorderStrong]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);   // Prefer using Alpha=1.0 here
            colors[ImGuiCol_TableBorderLight]          = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);   // Prefer using Alpha=1.0 here
            colors[ImGuiCol_TableRowBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_TableRowBgAlt]             = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
            colors[ImGuiCol_TextLink]                  = colors[ImGuiCol_HeaderActive];
            colors[ImGuiCol_TextSelectedBg]            = ImVec4(0.55f, 0.55f, 0.55f, 0.35f);
            colors[ImGuiCol_TreeLines]                 = colors[ImGuiCol_Border];
            colors[ImGuiCol_DragDropTarget]            = ImVec4(0.93f, 0.93f, 0.93f, 0.90f);
            colors[ImGuiCol_UnsavedMarker]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_NavCursor]                 = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight]     = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg]         = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg]          = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Detail)]  = { 0.500f, 0.500f, 0.500f, 1.f };
            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Info)]    = { 0.667f, 0.667f, 0.667f, 1.f };
            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Warning)] = { 1.000f, 1.000f, 0.333f, 1.f };
            gLogLevelColors[static_cast<size_t>(Lumen::DebugLog::LogLevel::Error)]   = { 1.000f, 0.333f, 0.333f, 1.f };
        }
    }

    /// custom version of ImGui::RenderFrame with ImDrawFlags
    void RenderFrameFlags(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool borders, float rounding, ImDrawFlags flags)
    {
        ImGuiContext &g = *GImGui;
        ImGuiWindow *window = g.CurrentWindow;
        window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding, flags);
        const float border_size = g.Style.FrameBorderSize;
        if (borders && border_size > 0.0f)
        {
            window->DrawList->AddRect(ImVec2(p_min.x + 1, p_min.y + 1), ImVec2(p_max.x + 1, p_max.y + 1), ImGui::GetColorU32(ImGuiCol_BorderShadow), rounding, flags, border_size);
            window->DrawList->AddRect(p_min, p_max, ImGui::GetColorU32(ImGuiCol_Border), rounding, flags, border_size);
        }
    }

    /// custom version ImGui::ButtonEx with round value and draw flags
    bool ButtonDrawFlags(const char *label, const ImVec2 &size_arg, ImGuiButtonFlags button_flags, float round_val, ImDrawFlags draw_flags)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, round_val);
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        ImVec2 pos = window->DC.CursorPos;
        if ((button_flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
            pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
        ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

        // Render
        const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        ImGui::RenderNavCursor(bb, id);
        RenderFrameFlags(bb.Min, bb.Max, col, true, style.FrameRounding, draw_flags);

        if (g.LogEnabled)
            ImGui::LogSetNextTextDecoration("[", "]");
        ImGui::RenderTextClipped(ImVec2(bb.Min.x + style.FramePadding.x, bb.Min.y + style.FramePadding.y), ImVec2(bb.Max.x - style.FramePadding.x, bb.Max.y - style.FramePadding.y), label, NULL, &label_size, style.ButtonTextAlign, &bb);

        // Automatically close popups
        //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
        //    CloseCurrentPopup();

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
        ImGui::PopStyleVar();
        return pressed;
    }

    /// custom version ImGui::Button with round value and draw flags
    bool Button(const char *label, float round_val, ImDrawFlags draw_flags, const ImVec2 &size_arg)
    {
        return ButtonDrawFlags(label, size_arg, ImGuiButtonFlags_None, round_val, draw_flags);
    }
}
#endif
