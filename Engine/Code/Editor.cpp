//==============================================================================================================================================================================
/// \file
/// \brief     editor
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditor.h"
#include "lEditorView.h"
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
        /// settings version
        dword version = 0;

        /// view visibility
        bool view = true;

        /// log visibility
        bool log = true;
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

    /// reset layout
    void ResetLayout();
private:

    /// run main menu bar
    void RunMainMenuBar();

    /// run Status Bar
    void RunStatusBar();

    /// current viewport
    ImGuiViewport *mViewport;

    /// main dockspace id
    ImGuiID mDockMainId;

    /// cached settings
    Settings mSettings;

    /// application pointer
    ApplicationWeakPtr mApplication;

    /// app view
    EditorViewPtr mEditorView;

    /// app log
    EditorLogPtr mEditorLog;
};

/// constructs editor
Editor::Impl::Impl(const ApplicationWeakPtr &application) :
    mViewport(nullptr), mDockMainId(0), mApplication(application), mEditorView(EditorView::MakePtr()), mEditorLog(EditorLog::MakePtr()) {}

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
                    mSettings.view = inEditorSettings.value("View", mSettings.view);
                    mSettings.log = inEditorSettings.value("Log", mSettings.log);
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

    // apply settings
    mEditorView->Show(mSettings.view);
    mEditorLog->Show(mSettings.log);
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
            outEditorSettings["View"] = mEditorView->Visible();
            outEditorSettings["Log"] = mEditorLog->Visible();
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

    mViewport = ImGui::GetMainViewport();
    ImGui::SetCurrentViewport(nullptr, (ImGuiViewportP *)mViewport); // Set viewport explicitly so GetFrameHeight reacts to DPI changes
    mDockMainId = ImGui::DockSpaceOverViewport();

    if (sNeedLayoutSetup)
    {
        sNeedLayoutSetup = false;
        ResetLayout();
    }

    RunMainMenuBar();
    mEditorView->Run("View", engine);
    mEditorLog->Run("Log");
    RunStatusBar();
}

/// reset layout
void Editor::Impl::ResetLayout()
{
    ImGuiID dockDownId;

    // clear existing layout
    ImGui::DockBuilderRemoveNode(mDockMainId);

    // add the main node with DockSpace and PassthruCentralNode flags, allows to see your background behind the docking system if no window is docked there
    ImGui::DockBuilderAddNode(mDockMainId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::DockBuilderSetNodeSize(mDockMainId, mViewport->Size);

    // split the dockspace into 2 nodes: upper and lower, mDockMainId itself gets updated to represent the remaining (Central) area
    ImGui::DockBuilderSplitNode(mDockMainId, ImGuiDir_Down, 0.2f, &dockDownId, &mDockMainId);

    // dock the windows
    ImGui::DockBuilderDockWindow("View", mDockMainId);
    ImGui::DockBuilderDockWindow("Log", dockDownId);

    ImGui::DockBuilderFinish(mDockMainId);

    mEditorView->Show(true);
    mEditorLog->Show(true);

    ImGui::SetWindowFocus("View");
}

/// run main menu bar
void Editor::Impl::RunMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Revert")) {}
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                if (auto application = mApplication.lock())
                {
                    application->Quit();
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Reset Layout"))
            {
                ResetLayout();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("View"))
            {
                mEditorView->Show(true);
                ImGui::SetWindowFocus("View");
            }
            if (ImGui::MenuItem("Log"))
            {
                mEditorLog->Show(true);
                ImGui::SetWindowFocus("Log");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

/// run status bar
void Editor::Impl::RunStatusBar()
{
    ImGui::BeginViewportSideBar("StatusBar", mViewport, ImGuiDir_Down, ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar();
    mEditorLog->PrintStatus();
    ImGui::EndMenuBar();
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
