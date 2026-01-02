//==============================================================================================================================================================================
/// \file
/// \brief     Editor Preferences
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditorPreferences.h"
#include "lImGuiLib.h"
#include "lEngine.h"

using namespace Lumen;

/// EditorPreferences::Impl class
class EditorPreferences::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs editor
    explicit Impl() : mWindowOpen(true), mThemeIndex(0) {}

    /// destructor
    ~Impl() {}

    void ShowThemeSelector(const char *label, Lumen::EnginePtr engine)
    {
        const char *themeNames[] = { "System", "Light", "Dark" };
        if (ImGui::BeginCombo(label, (mThemeIndex >= 0 && mThemeIndex < IM_ARRAYSIZE(themeNames)) ? themeNames[mThemeIndex] : ""))
        {
            for (int n = 0; n < IM_ARRAYSIZE(themeNames); n++)
            {
                if (ImGui::Selectable(themeNames[n], mThemeIndex == n, ImGuiSelectableFlags_SelectOnNav))
                {
                    SetTheme(n, engine);
                }
                else if (mThemeIndex == n)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    /// run editor preferences
    void Run(const char *title, Lumen::EnginePtr engine)
    {
        if (mWindowOpen)
        {
            ImGui::SetNextWindowSize(ImVec2(500, 110), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin(title, &mWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing))
            {
                ImGui::End();
                return;
            }
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);
            ImGui::SeparatorText("General");
            ShowThemeSelector("Theme##Selector", engine);
            ImGui::PopItemWidth();
            ImGui::End();
        }
    }

    /// return editor preferences window visibility
    bool Visible()
    {
        return mWindowOpen;
    }

    /// set editor preferences window visibility
    void Show(bool visible)
    {
        mWindowOpen = visible;
    }

    /// get theme
    int GetTheme()
    {
        return mThemeIndex;
    }

    /// set theme
    void SetTheme(int themeIndex, Lumen::EnginePtr engine)
    {
        mThemeIndex = themeIndex;
        switch (mThemeIndex)
        {
        case 0:
            Lumen::ImGuiLib::LumenTheme(engine->IsLightTheme());
            break;
        case 1:
            Lumen::ImGuiLib::LumenTheme(true);
            break;
        case 2:
            Lumen::ImGuiLib::LumenTheme(false);
            break;
        }
    }

private:
    /// window open
    bool mWindowOpen;

    /// current theme index
    int mThemeIndex = 0;
};

//==============================================================================================================================================================================

/// constructs editor
EditorPreferences::EditorPreferences() : mImpl(EditorPreferences::Impl::MakeUniquePtr()) {}

/// destructor
EditorPreferences::~EditorPreferences() {}

/// creates a smart pointer version of the editor log
EditorPreferencesPtr EditorPreferences::MakePtr()
{
    return EditorPreferencesPtr(new EditorPreferences());
}

/// run editor preferences
void EditorPreferences::Run(const char *title, Lumen::EnginePtr engine)
{
    mImpl->Run(title, engine);
}

/// return editor preferences window visibility
bool EditorPreferences::Visible()
{
    return mImpl->Visible();
}

/// set editor preferences window visibility
void EditorPreferences::Show(bool visible)
{
    mImpl->Show(visible);
}

/// get theme
int EditorPreferences::GetTheme()
{
    return mImpl->GetTheme();
}

/// set theme
void EditorPreferences::SetTheme(int themeIndex, Lumen::EnginePtr engine)
{
    mImpl->SetTheme(themeIndex, engine);
}
#endif
