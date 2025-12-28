#ifdef EDITOR

#include "lEditorView.h"
#include "lImGuiLib.h"
#include "lEngine.h"

using namespace Lumen;

/// EditorView::Impl class
class EditorView::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs editor
    explicit Impl() : mWindowOpen(true) {}

    /// destructor
    ~Impl() {}

    /// run editor view
    void Run(const char *title, Lumen::EnginePtr engine)
    {
        if (mWindowOpen)
        {
            if (!ImGui::Begin(title, &mWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing))
            {
                ImGui::End();
                return;
            }

            // draw a toolbar
            bool paused = false;
            if (auto application = engine->GetApplication().lock())
            {
                paused = application->Paused();
                if (paused)
                {
                    if (ImGui::Button("Play"))
                    {
                        application->Pause(false);
                    }
                }
                else
                {
                    if (ImGui::Button("Pause"))
                    {
                        application->Pause(true);
                    }
                }
                ImGui::SameLine();
            }

            // combo box to switch render targets
            static int currentView = 0; // 0 - RenderTexture / 1 - DepthStencil
            const char *views[] = { "RenderTexture", "DepthStencil" };

            // calc the width required by the combo box
            float maxWidth = 0;
            for (int i = 0; i < IM_ARRAYSIZE(views); i++)
            {
                float width = ImGui::CalcTextSize(views[i]).x;
                if (width > maxWidth) maxWidth = width;
            }
            float totalWidth = maxWidth + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetFrameHeight();

            // draw the combo box
            ImGui::SetNextItemWidth(totalWidth);
            if (ImGui::Combo("##View", &currentView, views, IM_ARRAYSIZE(views))) {}

            // draw the render texture in the remaining space
            Id::Type texId = static_cast<Id::Type>(currentView);
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            engine->SetRenderTextureSize(texId, { static_cast<int>(viewportPanelSize.x), static_cast<int>(viewportPanelSize.y) });
            ImGui::Image(engine->GetRenderTextureHandle(texId), viewportPanelSize);

            ImGui::End();
        }
    }

    /// return editor view visibility
    bool Visible()
    {
        return mWindowOpen;
    }

    /// set editor view visibility
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
EditorView::EditorView() : mImpl(EditorView::Impl::MakeUniquePtr()) {}

/// destructor
EditorView::~EditorView() {}

/// creates a smart pointer version of the editor log
EditorViewPtr EditorView::MakePtr()
{
    return EditorViewPtr(new EditorView());
}

/// run editor view
void EditorView::Run(const char *title, Lumen::EnginePtr engine)
{
    mImpl->Run(title, engine);
}

/// return editor view visibility
bool EditorView::Visible()
{
    return mImpl->Visible();
}

/// set editor view visibility
void EditorView::Show(bool visible)
{
    mImpl->Show(visible);
}
#endif
