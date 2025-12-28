//==============================================================================================================================================================================
/// \file
/// \brief     editor
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditor.h"
#include "lEditorLog.h"
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
    /// settings version
    inline static constexpr dword sSettingsVersion = 0x0001;

    /// editor settings
    struct Settings
    {
        dword version = 0;
    };

    /// constructs editor
    explicit Impl(const ApplicationWeakPtr &application);

    /// destructor
    ~Impl();

    /// initialize editor
    void Initialize();

    /// shutdown aplication
    void Shutdown();

    /// log callback
    void LogCallback(DebugLog::LogLevel level, std::string_view message);

    /// run editor
    void Run();

private:
    /// cached settings
    Settings mSettings;

    /// application pointer
    ApplicationWeakPtr mApplication;

    /// app log data
    EditorLogPtr mEditorLog;
};

/// constructs editor
Editor::Impl::Impl(const ApplicationWeakPtr &application) : mApplication(application), mEditorLog(EditorLog::MakePtr()) {}

/// destroys editor
Editor::Impl::~Impl() {}

/// initialize editor
void Editor::Impl::Initialize()
{
    // use lambda to forward log messages to our callback
    Lumen::DebugLog::SetCallback([this](DebugLog::LogLevel level, std::string_view msg) { this->LogCallback(level, msg); });

    // setup settings path
    std::filesystem::path inFile = "Settings/User.settings";
    Lumen::DebugLog::Info("Editor::Impl::Initialize loading {}", inFile.string());

    if (!std::filesystem::exists(inFile))
    {
        Lumen::DebugLog::Warning("Editor::Impl::Initialize user settings file does not exist, {}", inFile.string());
        return;
    }

    try
    {
        if (auto application = mApplication.lock())
        {
            if (auto engine = application->GetEngine().lock())
            {
                // set engine settings
                Engine::Settings engineSettings = engine->GetSettings();
                std::ifstream file(inFile);
                Serialized::Type in;
                file >> in;
                if (in.contains("Engine") && in["Engine"].is_object())
                {
                    Serialized::Type inEngineSettings = in["Engine"];
                    engineSettings.posX = inEngineSettings.value("PosX", engineSettings.posX);
                    engineSettings.posY = inEngineSettings.value("PosY", engineSettings.posY);
                    engineSettings.width = inEngineSettings.value("Width", engineSettings.width);
                    engineSettings.height = inEngineSettings.value("Height", engineSettings.height);
                    engineSettings.isMaximized = inEngineSettings.value("Maximized", engineSettings.isMaximized);
                    engineSettings.imGuiIni = inEngineSettings.value("ImGuiIni", engineSettings.imGuiIni);
                }
                engine->SetSettings(engineSettings);

                // set editor settings
                if (in.contains("Editor") && in["Editor"].is_object())
                {
                    Serialized::Type inEditorSettings = in["Editor"];
                    mSettings.version = inEditorSettings.value("Version", mSettings.version);
                }

                // versioning conversion
                if (mSettings.version != sSettingsVersion)
                {
                    Lumen::DebugLog::Warning("Editor::Impl::Initialize Settings needs conversion: 0x{:08X} to 0x{:08X}", mSettings.version, sSettingsVersion);
                    mSettings.version = sSettingsVersion;
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
            Engine::Settings engineSettings = engine->GetSettings();
            Serialized::Type outEngineSettings = {};
            outEngineSettings["PosX"] = engineSettings.posX;
            outEngineSettings["PosY"] = engineSettings.posY;
            outEngineSettings["Width"] = engineSettings.width;
            outEngineSettings["Height"] = engineSettings.height;
            outEngineSettings["Maximized"] = engineSettings.isMaximized;
            outEngineSettings["ImGuiIni"] = engineSettings.imGuiIni;
            out["Engine"] = outEngineSettings;

            Serialized::Type outEditorSettings = {};
            outEditorSettings["Version"] = mSettings.version;
            out["Editor"] = outEditorSettings;
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

/// log callback
void Editor::Impl::LogCallback(DebugLog::LogLevel level, std::string_view message)
{
    mEditorLog->AddMessage(level, message);
}

static void MainMenuBar_Run()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Revert")) {}
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Quit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static void Camera_Run(Lumen::EnginePtr engine)
{
    static bool sCameraWindowOpen = true;
    if (sCameraWindowOpen)
    {
        ImGui::Begin("Camera", &sCameraWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing);

        // draw a toolbar
        if (ImGui::Button("Play")) { /* Start Game Logic */ }
        ImGui::SameLine(); // Keep on same line
        if (ImGui::Button("Stop")) { /* Stop Game Logic */ }
        ImGui::SameLine();

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

static void StatusBar_Run(ImGuiViewport *viewport, ImVec4 color, const char *text)
{
    ImGui::BeginViewportSideBar("StatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar();
    ImGui::TextColored(color, text);
    ImGui::EndMenuBar();
    ImGui::End();
}

/// run editor
void Editor::Impl::Run()
{
    Lumen::ApplicationPtr application;
    Lumen::EnginePtr engine;
    if (application = mApplication.lock())
    {
        engine = application->GetEngine().lock();
    }
    if (!engine)
    {
        return;
    }
    static bool sNeedLayoutSetup = (!mSettings.version);

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetCurrentViewport(nullptr, (ImGuiViewportP *)viewport); // Set viewport explicitly so GetFrameHeight reacts to DPI changes
    ImGuiID dockMainId = ImGui::DockSpaceOverViewport();

    MainMenuBar_Run();
    Camera_Run(engine);
    mEditorLog->Run("Log");
    StatusBar_Run(viewport, { 1.f, 1.f, 1.f, 1.f }, "status bar");

    if (sNeedLayoutSetup)
    {
        sNeedLayoutSetup = false;
        ImGuiID dockDownId;

        // clear existing layout
        ImGui::DockBuilderRemoveNode(dockMainId);

        // add the main node with DockSpace and PassthruCentralNode flags, allows to see your background behind the docking system if no window is docked there
        ImGui::DockBuilderAddNode(dockMainId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::DockBuilderSetNodeSize(dockMainId, viewport->Size);

        // split the dockspace into 2 nodes: upper and lower, dockMainId itself gets updated to represent the remaining (Central) area
        ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.2f, &dockDownId, &dockMainId);

        // dock the windows
        ImGui::DockBuilderDockWindow("Camera", dockMainId);
        ImGui::DockBuilderDockWindow("Log", dockDownId);

        ImGui::DockBuilderFinish(dockMainId);
    }
}

//==============================================================================================================================================================================

/// constructs editor
Editor::Editor(const ApplicationWeakPtr &application) : mImpl(Editor::Impl::MakeUniquePtr(application)) {}

/// virtual destructor
Editor::~Editor() {}

/// creates a smart pointer version of the editor
EditorPtr Editor::MakePtr(const ApplicationWeakPtr &application)
{
    return EditorPtr(new Editor(application));
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
