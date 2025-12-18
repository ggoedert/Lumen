//==============================================================================================================================================================================
/// \file
/// \brief     editor
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditor.h"
#include "lImGuiLib.h"
#include "lCamera.h"
#include "lSceneManager.h"

using namespace Lumen;

/// Editor::Impl class
class Editor::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Editor;

public:
    /// constructs editor
    explicit Impl();

    /// destructor
    ~Impl();

    /// set application
    void SetApplication(const ApplicationWeakPtr &application);

    /// initialize editor
    void Initialize();

    /// shutdown aplication
    void Shutdown();

    /// run editor
    void Run();

private:
    /// application pointer
    ApplicationWeakPtr mApplication;
};

/// constructs editor
Editor::Impl::Impl() {}

/// destroys editor
Editor::Impl::~Impl() {}

/// set application
void Editor::Impl::SetApplication(const ApplicationWeakPtr &application)
{
    mApplication = application;
}

/// initialize editor
void Editor::Impl::Initialize() {}

/// shutdown aplication
void Editor::Impl::Shutdown() {}

/// run editor
void Editor::Impl::Run()
{
#ifdef SHOW_DEMO_WINDOW
    ImGui::ShowDemoWindow();
#else
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");                           // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");                // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window);       // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button"))                             // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
#endif
}

//==============================================================================================================================================================================

/// constructs editor
Editor::Editor() : mImpl(Editor::Impl::MakeUniquePtr()) {}

/// virtual destructor
Editor::~Editor() {}

/// creates a smart pointer version of the editor
EditorPtr Editor::MakePtr()
{
    return std::make_shared<Editor>();
}

/// set application
void Editor::SetApplication(const ApplicationWeakPtr &application)
{
    mImpl->SetApplication(application);
}

/// initialize editor
void Editor::Initialize()
{
    mImpl->Initialize();
}

/// shutdown aplication
void Editor::Shutdown()
{
    mImpl->Shutdown();
}

/// run editor
void Editor::Run()
{
    mImpl->Run();
}
#endif
