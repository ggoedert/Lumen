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

    /// current layout version
    const dword mLayoutVersion;
};

/// constructs editor
Editor::Impl::Impl(const ApplicationWeakPtr &application) : mApplication(application), mLayoutVersion(0x0001) {}

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
        if (auto application = mApplication.lock())
        {
            if (auto engine = application->GetEngine().lock())
            {
                Engine::LayoutSettings layoutSettings = engine->GetLayoutSettings();
                dword currentVersion = 0x0;
                std::ifstream file(inFile);
                Serialized::Type in;
                file >> in;
                if (in.contains("LayoutSettings") && in["LayoutSettings"].is_object())
                {
                    Serialized::Type inLayoutSettings = in["LayoutSettings"];
                    layoutSettings.posX = inLayoutSettings.value("PosX", layoutSettings.posX);
                    layoutSettings.posY = inLayoutSettings.value("PosY", layoutSettings.posY);
                    layoutSettings.width = inLayoutSettings.value("Width", layoutSettings.width);
                    layoutSettings.height = inLayoutSettings.value("Height", layoutSettings.height);
                    layoutSettings.isMaximized = inLayoutSettings.value("Maximized", layoutSettings.isMaximized);
                    layoutSettings.imGuiIni = inLayoutSettings.value("ImGuiIni", layoutSettings.imGuiIni);
                    layoutSettings.appData = inLayoutSettings.value("AppData", layoutSettings.appData);
                    std::vector<byte> decoded = Base64DecodeBytes(layoutSettings.appData);
                    if (decoded.size() >= sizeof(mLayoutVersion))
                    {
                        std::memcpy(&currentVersion, decoded.data(), sizeof(mLayoutVersion));
                    }
                }
                // ?? any versioning conversion ??
                if (currentVersion != mLayoutVersion)
                {
                    Lumen::DebugLog::Warning("Editor::Impl::Initialize LayoutSettings needs conversion: 0x{:08X} to 0x{:08X}", currentVersion, mLayoutVersion);
                    layoutSettings.appData = Base64EncodeBytes(
                        std::vector<byte>(
                            reinterpret_cast<const byte *>(&mLayoutVersion),
                            reinterpret_cast<const byte *>(&mLayoutVersion) + sizeof(mLayoutVersion)
                        )
                    );
                }
                // ?? any versioning conversion ??
                engine->SetLayoutSettings(layoutSettings);
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
            outLayoutSettings["AppData"] = Base64EncodeBytes(
                std::vector<byte>(
                    reinterpret_cast<const byte *>(&mLayoutVersion),
                    reinterpret_cast<const byte *>(&mLayoutVersion) + sizeof(mLayoutVersion)
                )
            );
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

static void ShowMainMenuBar()
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
static void ShowCamera(Lumen::EnginePtr engine)
{
    if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing))
    {
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
    }
    ImGui::End();
}

static void ShowLog()
{
    ImGui::Begin("Log", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::Text("Current Log");
    ImGui::End();
}

static void ShowStatusBar(ImGuiViewport *viewport, ImVec4 color, const char *text)
{
    if (ImGui::BeginViewportSideBar("StatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            ImGui::TextColored(color, text);
            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
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
    static bool sNeedLayoutSetup = engine->GetLayoutSettings().appData.empty();

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetCurrentViewport(nullptr, (ImGuiViewportP *)viewport); // Set viewport explicitly so GetFrameHeight reacts to DPI changes
    ImGuiID dockMainId = ImGui::DockSpaceOverViewport();

    ShowMainMenuBar();
    ShowCamera(engine);
    ShowLog();
    ShowStatusBar(viewport, { 1.f, 1.f, 1.f, 1.f }, "status bar");

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
