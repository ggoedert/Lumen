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

/// Lumen ImGuiLib namespace
namespace Lumen::ImGuiLib
{
    /// material icons font setup
    ImFont *gMaterialIconsFont = nullptr;
    const char *gMaterialIconsFontFilename = "Data\\material-design-icons\\font\\MaterialIcons-Regular.ttf";
    ImWchar gMaterialIconsIconsRanges[] = { 0xE000, 0xF8FF, 0 };
    float gMaterialIconsFontSize = 16.f;
    float gMaterialIconsIconsGlyphOffset = (gMaterialIconsFontSize-6.f)*0.5f;

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
