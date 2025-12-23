//==============================================================================================================================================================================
/// \file
/// \brief     editor
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditor.h"
#include "lImGuiLib.h"
#include "lEngine.h"

#include <fstream>

using namespace Lumen;

/// Editor::Impl class
class Editor::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Editor;

public:
    /// constructs editor
    explicit Impl(const ApplicationWeakPtr &application);

    /// destructor
    ~Impl();

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
Editor::Impl::Impl(const ApplicationWeakPtr &application) : mApplication(application) {}

/// destroys editor
Editor::Impl::~Impl() {}

/// initialize editor
void Editor::Impl::Initialize()
{
    // load user settings
    std::filesystem::path inFile = "Settings/User.settings";
    Lumen::DebugLog::Info("Editor::Impl::Initialize loading {}", inFile.string());

    if (!std::filesystem::exists(inFile))
    {
        Lumen::DebugLog::Warning("Editor::Impl::Initialize user settings file does not exist, {}", inFile.string());
        return;
    }

    try
    {
        std::ifstream file(inFile);
        Serialized::Type in;
        file >> in;

        if (auto application = mApplication.lock())
        {
            if (auto engine = application->GetEngine().lock())
            {
                if (in.contains("LayoutSettings") && in["LayoutSettings"].is_object())
                {
                    Serialized::Type inLayoutSettings = in["LayoutSettings"];
                    Engine::LayoutSettings layoutSettings;
                    layoutSettings.posX = inLayoutSettings.value("PosX", 0);
                    layoutSettings.posY = inLayoutSettings.value("PosY", 0);
                    layoutSettings.width = inLayoutSettings.value("Width", 0);
                    layoutSettings.height = inLayoutSettings.value("Height", 0);
                    layoutSettings.isMaximized = inLayoutSettings.value("Maximized", false);
                    layoutSettings.imGuiIni = inLayoutSettings.value("ImGuiIni", std::vector<std::string>());
                    engine->SetLayoutSettings(layoutSettings);
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        Lumen::DebugLog::Error("Unable to open user settings file for reading, {}", e.what());
    }
}

/// shutdown aplication
void Editor::Impl::Shutdown()
{
    std::filesystem::path outFile = "Settings/User.settings";
    Lumen::DebugLog::Info("Editor::Impl::Shutdown saving {}", outFile.string());

    Serialized::Type out;
    if (auto application = mApplication.lock())
    {
        if (auto engine = application->GetEngine().lock())
        {
            Engine::LayoutSettings layoutSettings = engine->GetLayoutSettings();
            Serialized::Type outLayoutSettings = {};
            outLayoutSettings["PosX"] = layoutSettings.posX;
            outLayoutSettings["PosY"] = layoutSettings.posY;
            outLayoutSettings["Width"] = layoutSettings.width;
            outLayoutSettings["Height"] = layoutSettings.height;
            outLayoutSettings["Maximized"] = layoutSettings.isMaximized;
            outLayoutSettings["ImGuiIni"] = layoutSettings.imGuiIni;
            out["LayoutSettings"] = outLayoutSettings;
        }
    }
    std::ofstream file(outFile);
    if (!file.is_open())
    {
        Lumen::DebugLog::Error("Unable to open user settings file for writing, {}", outFile.string());
        return;
    }
    file << out.dump(4);
    file.close();
}

/// run editor
void Editor::Impl::Run()
{
    Lumen::EnginePtr engine;
    if (auto application = mApplication.lock())
    {
        engine = application->GetEngine().lock();
    }
    if (!engine)
    {
        return;
    }

    ImGui::DockSpaceOverViewport();

#define SHOW_DEMO_WINDOW
#ifdef SHOW_DEMO_WINDOW
    ImGui::ShowDemoWindow();
#endif

    if (ImGui::Begin("Hello, world!"))                           // Create a window called "Hello, world!" and append into it.
    {
        static bool show_demo_window = true;
        static bool show_another_window = false;
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        static float f = 0.0f;
        static int counter = 0;

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
    }
    ImGui::End();

    if (ImGui::Begin("Game Viewport"))
    {
        Id::Type texId = 0; // 0 - RenderTexture / 1 - DepthStencil
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        engine->SetImRenderTextureSize(texId, viewportPanelSize);
        ImGui::Image(engine->GetImRenderTextureID(texId), viewportPanelSize);
    }
    ImGui::End();
}

//==============================================================================================================================================================================

/// constructs editor
Editor::Editor(const ApplicationWeakPtr &application) : mImpl(Editor::Impl::MakeUniquePtr(application)) {}

/// virtual destructor
Editor::~Editor() {}

/// creates a smart pointer version of the editor
EditorPtr Editor::MakePtr(const ApplicationWeakPtr &application)
{
    return std::make_shared<Editor>(application);
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
