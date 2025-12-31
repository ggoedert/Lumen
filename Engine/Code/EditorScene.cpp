//==============================================================================================================================================================================
/// \file
/// \brief     Editor Scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditorScene.h"
#include "lImGuiLib.h"
#include "lEngine.h"

using namespace Lumen;

/// EditorScene::Impl class
class EditorScene::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs editor
    explicit Impl() : mWindowOpen(true) {}

    /// destructor
    ~Impl() {}

    /// run editor scene
    void Run(const char *title, Lumen::EnginePtr engine)
    {
        if (mWindowOpen)
        {
            if (!ImGui::Begin(title, &mWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing))
            {
                ImGui::End();
                return;
            }

            // combo box to switch render targets
            static int currentRenderTarget = 0; // 0 - RenderTexture / 1 - DepthStencil
            const char *renderTargets[] = { "RenderTexture", "DepthStencil" };

            // calc the width required by the combo box
            float maxWidth = 0;
            for (int i = 0; i < IM_ARRAYSIZE(renderTargets); i++)
            {
                float width = ImGui::CalcTextSize(renderTargets[i]).x;
                if (width > maxWidth) maxWidth = width;
            }
            float totalWidth = maxWidth + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetFrameHeight();

            // draw the combo box
            ImGui::SetNextItemWidth(totalWidth);
            if (ImGui::Combo("##RenderTarget", &currentRenderTarget, renderTargets, IM_ARRAYSIZE(renderTargets))) {}

            // draw the render texture in the remaining space
            Id::Type texId = static_cast<Id::Type>(currentRenderTarget);
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            engine->SetRenderTextureSize(texId, { static_cast<int>(viewportPanelSize.x), static_cast<int>(viewportPanelSize.y) });
            ImGui::Image(engine->GetRenderTextureHandle(texId), viewportPanelSize);

            ImGui::End();
        }
    }

    /// return editor scene window visibility
    bool Visible()
    {
        return mWindowOpen;
    }

    /// set editor scene window visibility
    void Show(bool visible)
    {
        mWindowOpen = visible;
    }

private:
    /// window open
    bool mWindowOpen;
};

//==============================================================================================================================================================================

/// constructs editor
EditorScene::EditorScene() : mImpl(EditorScene::Impl::MakeUniquePtr()) {}

/// destructor
EditorScene::~EditorScene() {}

/// creates a smart pointer version of the editor log
EditorScenePtr EditorScene::MakePtr()
{
    return EditorScenePtr(new EditorScene());
}

/// run editor scene
void EditorScene::Run(const char *title, Lumen::EnginePtr engine)
{
    mImpl->Run(title, engine);
}

/// return editor scene window visibility
bool EditorScene::Visible()
{
    return mImpl->Visible();
}

/// set editor scene window visibility
void EditorScene::Show(bool visible)
{
    mImpl->Show(visible);
}
#endif
