//==============================================================================================================================================================================
/// \file
/// \brief     Editor
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lEditor.h"
#include "lEditorPreferences.h"
#include "lEditorScene.h"
#include "lEditorContent.h"
#include "lEditorLog.h"
#include "lImGuiLib.h"
#include "lEngine.h"
#include "lAsset.h"

/// \cond
#include <fstream>
/// \endcond

using namespace Lumen;

/// Editor::Impl class
class Editor::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Editor;

public:
    /// settings version
    inline static constexpr dword SettingsVersion = 0x0001;

    /// editor settings
    struct Settings
    {
        /// settings version
        dword version = 0;

        /// scene window visibility
        bool scene = true;

        /// content window visibility
        bool content = true;

        /// log window visibility
        bool log = true;

        /// theme index
        int theme = 0;
    };

    /// constructs editor
    explicit Impl(const ApplicationWeakPtr &application);

    /// destructor
    ~Impl();

    /// initialize editor
    void Initialize();

    /// shutdown editor
    void Shutdown();

    /// log callback
    void LogCallback(DebugLog::LogLevel level, std::string_view message);

    /// editor first run
    void FirstRun();

    /// process asset changes
    void ProcessAssetChanges(std::vector<FileSystem::AssetChange> &&assetBatch);

    /// run editor
    void Run();

    /// reset layout
    void ResetLayout();
private:

    /// run top bar
    void RunTopBar();

    /// run status Bar
    void RunStatusBar();

    /// current viewport
    ImGuiViewport *mViewport;

    /// main dockspace id
    ImGuiID mDockMainId;

    /// cached settings
    Settings mSettings;

    /// application pointer
    ApplicationWeakPtr mApplication;

    /// app preferences
    EditorPreferencesPtr mEditorPreferences;

    /// app scene
    EditorScenePtr mEditorScene;

    /// app content
    EditorContentPtr mEditorContent;

    /// app log
    EditorLogPtr mEditorLog;
};

/// constructs editor
Editor::Impl::Impl(const ApplicationWeakPtr &application) :
    mViewport(nullptr), mDockMainId(0), mApplication(application), mEditorPreferences(EditorPreferences::MakePtr()), mEditorScene(EditorScene::MakePtr())
{
    Lumen::ApplicationPtr applicationLock;
    Lumen::EnginePtr engine;
    if (applicationLock = mApplication.lock())
    {
        engine = applicationLock->GetEngine().lock();
    }
    if (!engine)
    {
        throw std::runtime_error("Editor::Impl::Impl no valid application or engine");
    }
    mEditorContent = EditorContent::MakePtr();
    mEditorLog = EditorLog::MakePtr(engine->GetExecutableName() + ".log");
}

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
                    engineSettings.mPosX = inEngineSettings.value("PosX", engineSettings.mPosX);
                    engineSettings.mPosY = inEngineSettings.value("PosY", engineSettings.mPosY);
                    engineSettings.mWidth = inEngineSettings.value("Width", engineSettings.mWidth);
                    engineSettings.mHeight = inEngineSettings.value("Height", engineSettings.mHeight);
                    engineSettings.mIsMaximized = inEngineSettings.value("Maximized", engineSettings.mIsMaximized);
                    engineSettings.mImGuiIni = inEngineSettings.value("ImGuiIni", engineSettings.mImGuiIni);
                }
                engine->SetSettings(engineSettings);

                // set editor settings
                if (in.contains("Editor") && in["Editor"].is_object())
                {
                    Serialized::Type inEditorSettings = in["Editor"];
                    mSettings.version = inEditorSettings.value("Version", mSettings.version);
                    mSettings.scene = inEditorSettings.value(EditorScene::Name(), mSettings.scene);
                    mSettings.content = inEditorSettings.value(EditorContent::Name(), mSettings.content);
                    mSettings.log = inEditorSettings.value(EditorLog::Name(), mSettings.log);
                    mSettings.theme = inEditorSettings.value("Theme", mSettings.theme);
                }

                // versioning conversion
                if (mSettings.version != SettingsVersion)
                {
                    Lumen::DebugLog::Warning("Editor::Impl::Initialize Settings needs conversion: 0x{:08X} to 0x{:08X}", mSettings.version, SettingsVersion);
                    mSettings.version = SettingsVersion;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        Lumen::DebugLog::Error("Unable to open user settings file for reading, {}", e.what());
    }
}

/// shutdown editor
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
            outEngineSettings["PosX"] = engineSettings.mPosX;
            outEngineSettings["PosY"] = engineSettings.mPosY;
            outEngineSettings["Width"] = engineSettings.mWidth;
            outEngineSettings["Height"] = engineSettings.mHeight;
            outEngineSettings["Maximized"] = engineSettings.mIsMaximized;
            outEngineSettings["ImGuiIni"] = engineSettings.mImGuiIni;
            out["Engine"] = outEngineSettings;

            Serialized::Type outEditorSettings = {};
            outEditorSettings["Version"] = mSettings.version;
            outEditorSettings[EditorScene::Name()] = mEditorScene->Visible();
            outEditorSettings[EditorContent::Name()] = mEditorContent->Visible();
            outEditorSettings[EditorLog::Name()] = mEditorLog->Visible();
            outEditorSettings["Theme"] = mEditorPreferences->GetTheme();
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

/// editor first run
void Editor::Impl::FirstRun()
{
    // apply settings
    mEditorPreferences->Show(false);
    mEditorScene->Show(mSettings.scene);
    mEditorContent->Show(mSettings.content);
    mEditorLog->Show(mSettings.log);

    // apply theme
    Lumen::ApplicationPtr application;
    Lumen::EnginePtr engine;
    if (application = mApplication.lock())
    {
        engine = application->GetEngine().lock();
    }
    L_ASSERT(engine);
    mEditorPreferences->SetTheme(mSettings.theme, engine);
}

/// process asset changes
void Editor::Impl::ProcessAssetChanges(std::vector<FileSystem::AssetChange> &&assetBatch)
{
    mEditorContent->ProcessAssetChanges(std::move(assetBatch));
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
    L_ASSERT(engine);
    static bool sNeedLayoutSetup = (!mSettings.version);

    mViewport = ImGui::GetMainViewport();
    ImGui::SetCurrentViewport(nullptr, (ImGuiViewportP *)mViewport); // Set viewport explicitly so GetFrameHeight reacts to DPI changes
    mDockMainId = ImGui::DockSpaceOverViewport();

    if (sNeedLayoutSetup)
    {
        sNeedLayoutSetup = false;
        ResetLayout();
    }

    RunTopBar();
    mEditorPreferences->Run(engine);
    mEditorScene->Run(engine);
    mEditorContent->Run();
    mEditorLog->Run();
    RunStatusBar();
}

/// reset layout
void Editor::Impl::ResetLayout()
{
    ImGuiID dockDownId;

    // clear existing layout
    ImGui::DockBuilderRemoveNode(mDockMainId);

    // add the main node with DockSpace and PassthruCentralNode flags, allows to see the background behind the docking system if no window is docked there
    ImGui::DockBuilderAddNode(mDockMainId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::DockBuilderSetNodeSize(mDockMainId, mViewport->Size);

    // split the dockspace into 2 nodes: upper and lower, mDockMainId itself gets updated to represent the remaining (Central) area
    ImGui::DockBuilderSplitNode(mDockMainId, ImGuiDir_Down, 0.2f, &dockDownId, &mDockMainId);

    // dock the windows
    ImGui::DockBuilderDockWindow(EditorScene::Name(), mDockMainId);
    ImGui::DockBuilderDockWindow(EditorContent::Name(), dockDownId);
    ImGui::DockBuilderDockWindow(EditorLog::Name(), dockDownId);

    ImGui::DockBuilderFinish(mDockMainId);

    mEditorPreferences->Show(false);
    mEditorScene->Show(true);
    mEditorContent->Show(true);
    mEditorLog->Show(true);

    ImGui::SetWindowFocus(EditorScene::Name());
}

/// run top bar
void Editor::Impl::RunTopBar()
{
    // main menu bar
    float menuBarHeight = 0.f;
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
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem(EditorPreferences::Name()))
            {
                mEditorPreferences->Show(true);
                ImGui::SetWindowFocus(EditorPreferences::Name());
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
            if (ImGui::MenuItem(EditorScene::Name()))
            {
                mEditorScene->Show(true);
                ImGui::SetWindowFocus(EditorScene::Name());
            }
            if (ImGui::MenuItem(EditorContent::Name()))
            {
                mEditorContent->Show(true);
                ImGui::SetWindowFocus(EditorContent::Name());
            }
            if (ImGui::MenuItem(EditorLog::Name()))
            {
                mEditorLog->Show(true);
                ImGui::SetWindowFocus(EditorLog::Name());
            }
            ImGui::EndMenu();
        }
        menuBarHeight = ImGui::GetCurrentWindow()->MenuBarHeight;
        ImGui::EndMainMenuBar();
    }

    // toolbar
    const float toolbarHeight = ImGuiLib::gMaterialIconsFontSize * 3.f;
    ImGui::PushFont(Lumen::ImGuiLib::gMaterialIconsFont, 0.f);
    ImGui::BeginViewportSideBar("ToolBar", mViewport, ImGuiDir_Up, toolbarHeight, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGuiLib::gMaterialIconsFontSize * 0.125f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGuiLib::gMaterialIconsFontSize * 0.75f, ImGuiLib::gMaterialIconsFontSize - 10.f));
    ImGui::SetCursorPosY((toolbarHeight - ImGui::GetFrameHeight()) * 0.5f);

    // calculate total width of the center buttons
    float playWidth = ImGui::CalcTextSize(MATERIAL_ICONS_PLAY).x + ImGui::GetStyle().FramePadding.x * 2.f;
    float pauseWidth = ImGui::CalcTextSize(MATERIAL_ICONS_PAUSE).x + ImGui::GetStyle().FramePadding.x * 2.f;
    float stepWidth = ImGui::CalcTextSize(MATERIAL_ICONS_STEP).x + ImGui::GetStyle().FramePadding.x * 2.f;
    float totalGroupWidth = playWidth + ImGui::GetStyle().ItemSpacing.x + pauseWidth + ImGui::GetStyle().ItemSpacing.x + stepWidth - ImGui::GetStyle().WindowPadding.x * 2.f;
    float centerX = (ImGui::GetContentRegionAvail().x - totalGroupWidth) * 0.5f;

    // side buttons
    if (ImGuiLib::Button(MATERIAL_ICONS_HAND, 5.f, ImDrawFlags_RoundCornersLeft)) {}
    ImGui::SameLine();
    if (ImGui::Button(MATERIAL_ICONS_MOVE)) {}
    ImGui::SameLine();
    if (ImGui::Button(MATERIAL_ICONS_ROTATE)) {}
    ImGui::SameLine();
    if (ImGuiLib::Button(MATERIAL_ICONS_SCALE, 5.f, ImDrawFlags_RoundCornersRight)) {}
    ImGui::SameLine();

    // center buttons
    if (auto application = mApplication.lock())
    {
        ImGui::SetCursorPosX(centerX);
        Application::State state = application->GetState();
        bool started = (state == Application::State::Running) || (state == Application::State::Stepped);
        bool paused = state == Application::State::Paused;
        if (started)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        }
        if (ImGuiLib::Button(MATERIAL_ICONS_PLAY, 5.f, ImDrawFlags_RoundCornersLeft))
        {
            if (started)
            {
                if (paused)
                {
                    application->Pause();
                }
                application->Stop();
            }
            else
            {
                application->Start();
            }
        }
        if (started)
        {
            ImGui::PopStyleColor(2);
        }
        ImGui::SameLine();
        if (paused)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        }
        if (ImGui::Button(MATERIAL_ICONS_PAUSE))
        {
            application->Pause();
        }
        if (paused)
        {
            ImGui::PopStyleColor(2);
        }
        ImGui::SameLine();

        bool canStep = started;
        ImGui::BeginDisabled(!canStep);
        if (ImGuiLib::Button(MATERIAL_ICONS_STEP, 5.f, ImDrawFlags_RoundCornersRight))
        {
            application->Step();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
    }

    ImGui::PopStyleVar(2);
    ImGui::End();
    ImGui::PopFont();
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

/// shutdown editor
void Editor::Shutdown()
{
    mImpl->Shutdown();
}

/// editor first run
void Editor::FirstRun()
{
    mImpl->FirstRun();
}

/// process asset changes
void Editor::ProcessAssetChanges(std::vector<FileSystem::AssetChange> &&assetBatch)
{
    mImpl->ProcessAssetChanges(std::move(assetBatch));
}

/// run editor
void Editor::Run()
{
    mImpl->Run();
}
#endif
