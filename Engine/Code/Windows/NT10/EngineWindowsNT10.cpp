//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows nt10 implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindows.h"
#include "lFolderFileSystem.h"
#include "lTexture.h"
#include "lShader.h"
#include "lMesh.h"
#include "lAssetManager.h"
#include "lEngine.h"

#include "EngineImpl.h"

// helpers headers
#include "EngineFramework.h"
#include "DeviceResources.h"
#include "DynamicDescriptorHeap.h"
#include "StepTimer.h"
#include "DDS.h"

#ifdef EDITOR
// ImGui editor support
#include "lImGuiLib.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

using namespace DX;
using namespace DirectX;
using namespace SimpleMath;
using namespace Lumen;

using Microsoft::WRL::ComPtr;

/// Lumen WindowsNT10 namespace
namespace Lumen::WindowsNT10
{
    /// Engine windows NT10 implementation.
    /// creates a D3D12 device and provides a game loop
    class EngineWindowsNT10 final : public Engine::Impl, public DX::IDeviceNotify
    {
        CLASS_NO_COPY_MOVE(EngineWindowsNT10);

    public:
        explicit EngineWindowsNT10();
        ~EngineWindowsNT10() override;

        /// set owner
        void SetOwner(EngineWeakPtr owner) override { mOwner = owner; }

        // initialization and management
        bool Initialize(const Object &config) override;

        /// create new resources
        bool CreateNewResources() override;

        /// shutdown
        void Shutdown() override;

        // get elapsed time since last run
        float GetElapsedTime() override;

        // basic game loop
        bool Run(std::function<bool()> update, std::function<void()> preRender) override;

        // IDeviceNotify
        void OnDeviceLost() override;
        void OnDeviceRestored() override;

        // messages
        void OnActivated();
        void OnDeactivated();
        void OnSuspending();
        void OnResuming();
        void OnWindowMoved();
        void OnDisplayChange();
        void OnWindowSizeChanged(int width, int height);

#ifdef EDITOR
        /// get layout settings
        Engine::LayoutSettings GetLayoutSettings() noexcept override;

        /// set layout settings
        void SetLayoutSettings(Engine::LayoutSettings &layoutSettings) noexcept override;
#endif

        // get fullscreen size
        void GetFullscreenSize(int &width, int &height) const noexcept override;

        static BYTE sBuffer[65536];
        static OVERLAPPED sOverlapped;
        static HANDLE sDirHandle;

        /// create a file system for the assets
        IFileSystemPtr AssetsFileSystem() const override
        {
            return FolderFileSystem::MakePtr("Assets");
        }

#ifdef EDITOR
        static void CALLBACK StaticFileChangeCallback(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped)
        {
            if (errorCode == ERROR_SUCCESS)
            {
                reinterpret_cast<EngineWindowsNT10 *>(overlapped->hEvent)->FileChangeCallback();
            }
            else
            {
                std::string message;
                LPSTR buffer = nullptr;
                DWORD size = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    errorCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&buffer,
                    0, nullptr);
                if (size != 0 && buffer != nullptr)
                {
                    message = buffer;
                    LocalFree(buffer);
                }
                else
                {
                    message = "Unknown error";
                }
                Lumen::DebugLog::Error("FileChange callback error: {}", message);  // OutputDebugStringA should be thread safe
            }

            // re-arm the watcher
            ReadDirectoryChangesW(
                sDirHandle,
                sBuffer,
                sizeof(sBuffer),
                TRUE, // recursive
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                NULL,
                overlapped,
                StaticFileChangeCallback
            );
        }

        void FileChangeCallback()
        {
            static DWORD sLastAction = -1;
            static double sLastTimer = -1.f;
            static std::string sLastFilename;
            static std::vector<Lumen::AssetManager::AssetChange> batch;

            /// get elapsed milliseconds since last callback
            double timer = mTimer.GetElapsedSeconds();
            double elapsedMiliseconds = (sLastTimer >= 0.f) ? (timer - sLastTimer) * 1000.f : FLT_MAX;
            sLastTimer = timer;

            FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION *)sBuffer;
            while (true)
            {
                std::wstring wfilename(info->FileName, info->FileNameLength / sizeof(WCHAR));
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), (int)wfilename.size(), nullptr, 0, nullptr, nullptr);
                std::string filename(size_needed, 0);
                WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), (int)wfilename.size(), filename.data(), size_needed, nullptr, nullptr);

                if ((elapsedMiliseconds >= 50.f) || (sLastAction != info->Action) || (sLastFilename != filename))
                {
                    switch (info->Action)
                    {
                    case FILE_ACTION_ADDED:
                        batch.push_back({ Lumen::AssetManager::AssetChange::Type::Added, filename, "" });
                        break;
                    case FILE_ACTION_MODIFIED:
                        batch.push_back({ Lumen::AssetManager::AssetChange::Type::Modified, filename, "" });
                        break;
                    case FILE_ACTION_RENAMED_NEW_NAME:
                        batch.push_back({ Lumen::AssetManager::AssetChange::Type::Renamed, filename, sLastFilename });
                        break;
                    case FILE_ACTION_REMOVED:
                        batch.push_back({ Lumen::AssetManager::AssetChange::Type::Removed, filename, "" });
                        break;
                    }
                    sLastFilename = filename;
                    sLastAction = info->Action;
                }

                if (info->NextEntryOffset == 0)
                {
                    break;
                }
                info = (FILE_NOTIFY_INFORMATION *) (((BYTE *)info) + info->NextEntryOffset);
            }
            if (!batch.empty())
            {
                if (auto owner = mOwner.lock())
                {
                    owner->PushAssetChangeBatch(std::move(batch));
                }
            }
        }
#endif

        /// begin scene
        void BeginScene() override;

        /// push render command
        void PushRenderCommand(Engine::RenderCommand renderCommand) override;

        /// end scene
        void EndScene() override;

        /// create a texture
        Id::Type CreateTexture(const TexturePtr &texture, int width, int height) override;

        /// release a texture
        void ReleaseTexture(Id::Type texId) override;

        /// create a shader
        Id::Type CreateShader(const ShaderPtr &shader) override;

        /// release a shader
        void ReleaseShader(Id::Type shaderId) override;

        /// create a mesh
        Id::Type CreateMesh(const MeshPtr &mesh) override;

        /// release a mesh
        void ReleaseMesh(Id::Type meshId) override;

        /// set render texture size
        void SetRenderTextureSize(Id::Type texId, Math::Int2 size) override;

        /// get render texture id
        qword GetRenderTextureHandle(Id::Type texId) override;

#ifdef EDITOR
        /// check if ImGui is initialized
        bool ImGuiInitialized() { return mImGuiInitialized; }
#endif

    private:
        void Render();

        void CreateNewDeviceDependentResources();
        void CreateNewWindowSizeDependentResources();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

        /// main window handle
        HWND mWindow = nullptr;

#ifdef EDITOR
        /// ImGui initialized
        bool mImGuiInitialized = false;

        /// ImGui monitor scale information
        float mMainScale = 1.f;
#endif

        // device resources
        std::unique_ptr<DX::DeviceResources> mDeviceResources;

        // rendering loop timer
        StepTimer mTimer;

        std::unique_ptr<GraphicsMemory> mGraphicsMemory;
        std::unique_ptr<CommonStates> mStates;

        SimpleMath::Matrix mView;
        SimpleMath::Matrix mProj;

        std::unique_ptr<DynamicDescriptorHeap> mResourceDescriptors;

        /// render target resources (Scene -> Texture)
        int mSceneWidth = 0;
        int mSceneHeight = 0;
        bool mSceneNeedsResize = false;
        Microsoft::WRL::ComPtr<ID3D12Resource> mSceneRenderTarget;
        Microsoft::WRL::ComPtr<ID3D12Resource> mSceneDepthStencil;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSceneRTVHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSceneDSVHeap;
        DynamicDescriptorHeap::IndexType mSceneSRVIndex = DynamicDescriptorHeap::InvalidIndex;
        DynamicDescriptorHeap::IndexType mSceneDSVIndex = DynamicDescriptorHeap::InvalidIndex;
        D3D12_RESOURCE_STATES mSceneState = D3D12_RESOURCE_STATE_COMMON;
        D3D12_RESOURCE_STATES mSceneDepthState = D3D12_RESOURCE_STATE_COMMON;

        /// texture id generator
        Id::Generator mTexIdGenerator;

        /// shader id generator
        Id::Generator mShaderIdGenerator;

        /// mesh id generator
        Id::Generator mMeshIdGenerator;

        /// map of mesh
        struct MeshData
        {
            MeshPtr mMesh;
            std::unique_ptr<GeometricPrimitive> mShape;
        };
        using MeshMapType = std::unordered_map<Id::Type, MeshData>;
        MeshMapType mMeshMap;
        std::vector<MeshMapType::iterator> mNewDeviceMeshMap;

        /// map of shader
        struct ShaderData
        {
            ShaderPtr mShader;
            std::unique_ptr<IEffect> mEffect;
        };
        using ShaderMapType = std::unordered_map<Id::Type, ShaderData>;
        ShaderMapType mShaderMap;
        std::vector<ShaderMapType::iterator> mNewDeviceShaderMap;
        std::vector<ShaderMapType::iterator> mNewWindowShaderMap;

        /// map of texture
        struct TextureData
        {
            TexturePtr mTexture;
            DynamicDescriptorHeap::IndexType mIndex = DynamicDescriptorHeap::InvalidIndex;
            Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
            int mWidth = 0;
            int mHeight = 0;
        };
        using TextureMapType = std::unordered_map<Id::Type, TextureData>;
        TextureMapType mTextureMap;
        std::vector<TextureMapType::iterator> mNewDeviceTextureMap;

        std::vector<Engine::RenderCommand> mRenderCommands;

#ifdef EDITOR
        /// cached layout settings
        Engine::LayoutSettings mLayoutSettings;
#endif

        /// owner
        EngineWeakPtr mOwner;
    };

    EngineWindowsNT10::EngineWindowsNT10() :
        mDeviceResources(std::make_unique<DeviceResources>())
    {
        // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
        //   Add DeviceResources::c_AllowTearing to opt-in to variable rate displays.
        //   Add DeviceResources::c_EnableHDR for HDR10 display.
        mDeviceResources->RegisterDeviceNotify(this);
    }

    EngineWindowsNT10::~EngineWindowsNT10()
    {
        if (mDeviceResources)
        {
            mDeviceResources->WaitForGpu();
        }
    }

    /// initialize the Direct3D resources required to run
    bool EngineWindowsNT10::Initialize(const Object &config)
    {
#ifdef EDITOR
        // make process DPI aware and obtain main monitor scale
        ImGui_ImplWin32_EnableDpiAwareness();
        mMainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT { 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
#endif

        if (config.Type() != Windows::Config::Type())
        {
#ifdef TYPEINFO
            DebugLog::Error("Initialize engine, unknown config type: {}", config.Type().mName);
#else
            DebugLog::Error("Initialize engine, unknown config hash type: 0x{:08X}", config.Type());
#endif
            return false;
        }

        const auto &initializeConfig = static_cast<const Windows::Config &>(config);
        mWindow = initializeConfig.mWindow;
#ifdef EDITOR
        /// get cached layout settings
        mLayoutSettings = GetLayoutSettings();
        mSceneWidth = mLayoutSettings.width;
        mSceneHeight = mLayoutSettings.height;
#else
        RECT rc;
        GetClientRect(mWindow, &rc);
        OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        mSceneWidth = rc.right - rc.left;
        mSceneHeight = rc.bottom - rc.top;
#endif
        mSceneNeedsResize = false;
        mDeviceResources->SetWindow(mWindow, mSceneWidth, mSceneHeight);

        mDeviceResources->CreateDeviceResources();
        CreateDeviceDependentResources();

        mDeviceResources->CreateWindowSizeDependentResources();
        CreateWindowSizeDependentResources();

#ifdef EDITOR
        // --------------------------------------------------------------
        //  Initialize Dear ImGui Resources
        // --------------------------------------------------------------

        // setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;                                 // Disable ini file
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // load ImGui settings
        std::string combinedImGuiIni;
        for (const auto &line : mLayoutSettings.imGuiIni)
        {
            combinedImGuiIni += line;
            combinedImGuiIni += '\n';
        }
        ImGui::LoadIniSettingsFromMemory(combinedImGuiIni.c_str(), combinedImGuiIni.size());

        // setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // setup scaling
        ImGuiStyle &style = ImGui::GetStyle();
        //style.ScaleAllSizes(mMainScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        //style.FontScaleDpi = mMainScale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
        io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
        io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

        // when viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(mWindow);

        // for backbuffer count
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        mDeviceResources->GetSwapChain()->GetDesc(&swapChainDesc);

        // Setup Dear ImGui DirectX12 backend
        ImGui_ImplDX12_InitInfo init_info = {};
        init_info.Device = mDeviceResources->GetD3DDevice();
        init_info.CommandQueue = mDeviceResources->GetCommandQueue();
        init_info.NumFramesInFlight = swapChainDesc.BufferCount;
        init_info.RTVFormat = mDeviceResources->GetBackBufferFormat();
        init_info.DSVFormat = mDeviceResources->GetDepthBufferFormat();
        // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
        // (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
        init_info.SrvDescriptorHeap = mResourceDescriptors->Heap();
        init_info.UserData = static_cast<void *>(mResourceDescriptors.get());
        init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *init_info, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle)
        {
            DynamicDescriptorHeap *resourceDescriptors = static_cast<DynamicDescriptorHeap *>(init_info->UserData);
            DynamicDescriptorHeap::IndexType index = resourceDescriptors->Allocate();
            *out_cpu_handle = resourceDescriptors->GetCpuHandle(index);
            *out_gpu_handle = resourceDescriptors->GetGpuHandle(index);
        };
        init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *init_info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
        {
            DynamicDescriptorHeap *resourceDescriptors = static_cast<DynamicDescriptorHeap *>(init_info->UserData);
            resourceDescriptors->Free(resourceDescriptors->GetIndex(cpu_handle, gpu_handle));
        };
        ImGui_ImplDX12_Init(&init_info);

        // mark ImGui initialized
        mImGuiInitialized = true;
#endif

        try
        {
            static std::vector<Lumen::AssetManager::AssetChange> batch;
            for (const auto &entry : std::filesystem::recursive_directory_iterator("Assets"))
            {
                if (entry.is_regular_file())
                {
                    std::string filename = entry.path().lexically_relative("Assets").generic_string();
                    batch.push_back({ Lumen::AssetManager::AssetChange::Type::Added, entry.path().generic_string(), "" });
                }
            }
            if (!batch.empty())
            {
                if (auto owner = mOwner.lock())
                {
                    owner->PushAssetChangeBatch(std::move(batch));
                }
            }
        }
        catch (const std::exception &e)
        {
            DebugLog::Error("Initialize engine, unable to process {} directory: {}", "Assets", e.what());
        }

#ifdef EDITOR
        std::string monitorDir = "Assets";
        sDirHandle = CreateFileA(
            monitorDir.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ |
            FILE_SHARE_WRITE |
            FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS |
            FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (sDirHandle != INVALID_HANDLE_VALUE)
        {
            sOverlapped = {};
            sOverlapped.hEvent = this; // store "this" so callback can find the object

            // start watching
            ReadDirectoryChangesW(
                sDirHandle,
                sBuffer,
                sizeof(sBuffer),
                TRUE, // recursive
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                NULL,
                &sOverlapped,
                StaticFileChangeCallback
            );
        }
        else
        {
            Lumen::DebugLog::Error("Could not open directory to monitor: {}", monitorDir);
        }

        // trigger initial window size changed to setup ImGui
        RECT rc;
        GetClientRect(mWindow, &rc);
        OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
#endif

        return true;
    }

    /// create resources
    bool EngineWindowsNT10::CreateNewResources()
    {
        CreateNewDeviceDependentResources();
        CreateNewWindowSizeDependentResources();
        return true;
    }

    /// shutdown
    void EngineWindowsNT10::Shutdown()
    {
#ifdef EDITOR
        // Shutdown ImGui first
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif

        if (mSceneSRVIndex != DynamicDescriptorHeap::InvalidIndex)
        {
            mResourceDescriptors->Free(mSceneSRVIndex);
            mSceneSRVIndex = DynamicDescriptorHeap::InvalidIndex;
        }
        if (mSceneDSVIndex != DynamicDescriptorHeap::InvalidIndex)
        {
            mResourceDescriptors->Free(mSceneDSVIndex);
            mSceneDSVIndex = DynamicDescriptorHeap::InvalidIndex;
        }
        mSceneRenderTarget.Reset();
        mSceneDepthStencil.Reset();

        mNewDeviceMeshMap.clear();
        mNewDeviceShaderMap.clear();
        mNewWindowShaderMap.clear();
        mNewDeviceTextureMap.clear();

        // release meshes
        auto mesKeyView = std::views::keys(mMeshMap);
        for (auto &meshId : std::vector<Id::Type>(mesKeyView.begin(), mesKeyView.end()))
        {
            auto it = mMeshMap.find(meshId);
            if (it != mMeshMap.end())
            {
                it->second.mMesh->Release();
            }
        }
        L_ASSERT(mMeshMap.empty());

        // release shaders
        auto shaKeyView = std::views::keys(mShaderMap);
        for (auto &shaderID : std::vector<Id::Type>(shaKeyView.begin(), shaKeyView.end()))
        {
            auto it = mShaderMap.find(shaderID);
            if (it != mShaderMap.end())
            {
                it->second.mShader->Release();
            }
        }
        L_ASSERT(mShaderMap.empty());

        // release textures
        auto texKeyView = std::views::keys(mTextureMap);
        for (auto &texId : std::vector<Id::Type>(texKeyView.begin(), texKeyView.end()))
        {
            auto it = mTextureMap.find(texId);
            if (it != mTextureMap.end())
            {
                it->second.mTexture->Release();
            }
        }
        L_ASSERT(mTextureMap.empty());
    }

    // get elapsed time since last run
    float EngineWindowsNT10::GetElapsedTime()
    {
        return static_cast<float>(mTimer.GetElapsedSeconds());
    }

#pragma region Frame Update
    /// executes the basic game loop
    bool EngineWindowsNT10::Run(std::function<bool()> update, std::function<void()> preRender)
    {
        bool updateResult = true;

        PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");
        mTimer.Tick([&]()
        {
            if (updateResult && update)
            {
                updateResult = update();
            }
        });
        PIXEndEvent();
        if (!updateResult)
            return false;

#ifdef EDITOR
        ImGui_ImplWin32_NewFrame();  // window/input
        ImGui_ImplDX12_NewFrame();   // renderer backend
        ImGui::NewFrame();
#endif

        if (preRender)
        {
            preRender();
        }
        Render();

#ifdef EDITOR
        // allow APC callbacks to run (e.g., file watcher)
        SleepEx(0, TRUE);
#endif
        return true;
    }
#pragma endregion

#pragma region Frame Render
    /// draws the scene
    void EngineWindowsNT10::Render()
    {
        // don't try to render anything before the first Update
        if (mTimer.GetFrameCount() == 0)
        {
            return;
        }

#ifdef EDITOR
        if (mSceneNeedsResize)
        {
            mDeviceResources->WaitForGpu(); // Critical: don't delete resources in use
            CreateWindowSizeDependentResources();
            mSceneNeedsResize = false;
        }
#endif

        // prepare the command list to render a new frame
        mDeviceResources->Prepare();
        auto commandList = mDeviceResources->GetCommandList();

        // get render target view and resource
        // this may be expanded for post effects later like Bloom, Motion Blur, or SSAO, with the use of multiple render targets/ping pong render targets
        D3D12_CPU_DESCRIPTOR_HANDLE targetRtv;
        D3D12_CPU_DESCRIPTOR_HANDLE stencilDsv;
        ID3D12Resource *targetResource;
        ID3D12Resource *stencilResource;
#ifdef EDITOR
        // render to the texture so ImGui can display it in a window
        targetRtv = mSceneRTVHeap->GetCPUDescriptorHandleForHeapStart();
        targetResource = mSceneRenderTarget.Get();
        stencilDsv = mSceneDSVHeap->GetCPUDescriptorHandleForHeapStart();
        stencilResource = mSceneDepthStencil.Get();
#else
        // render directly to the screen's backbuffer
        targetRtv = mDeviceResources->GetRenderTargetView();
        targetResource = mDeviceResources->GetRenderTarget();
        stencilDsv = mDeviceResources->GetDepthStencilView();
        stencilResource = mDeviceResources->GetDepthStencil();
#endif

        ID3D12DescriptorHeap *heaps[] = { mResourceDescriptors->Heap(), mStates->Heap() };
        commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

        // =================================================================================
        // PASS 1: Render 3D Scene to Texture
        // =================================================================================
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render Scene");
        {
#ifdef EDITOR
            // in editor mode, we MUST transition our custom texture to RENDER_TARGET
            // transition target to RENDER_TARGET state
            // in non-editor mode, DeviceResources::Prepare usually handles this,
            // but being explicit here ensures the logic is robust

            // transition Color Target to RENDER_TARGET
            TransitionResource(commandList, targetResource, mSceneState, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mSceneState = D3D12_RESOURCE_STATE_RENDER_TARGET;

            // transition Depth Target to DEPTH_WRITE
            TransitionResource(commandList, stencilResource, mSceneDepthState, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            mSceneDepthState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
#endif

            commandList->OMSetRenderTargets(1, &targetRtv, FALSE, &stencilDsv);

            // clear
            commandList->ClearRenderTargetView(targetRtv, Colors::CornflowerBlue, 0, nullptr);
            commandList->ClearDepthStencilView(stencilDsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

            // set viewport/scissor
#ifdef EDITOR
            //FIXME TESTING ONLY
            //we should add better multi render target support
            D3D12_VIEWPORT sceneViewport = { 0.0f, 0.0f, (float)mSceneWidth, (float)mSceneHeight, 0.0f, 1.0f };
            D3D12_RECT sceneScissor = { 0, 0, (long)mSceneWidth, (long)mSceneHeight };
            commandList->RSSetViewports(1, &sceneViewport);
            commandList->RSSetScissorRects(1, &sceneScissor);
            //FIXME TESTING ONLY
#else
            auto viewport = mDeviceResources->GetScreenViewport();
            auto scissorRect = mDeviceResources->GetScissorRect();
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);
#endif

            // draw 3d objects
            for (auto &cmd : mRenderCommands)
            {
                if (auto pDrawPrimitiveData = std::get_if<Engine::DrawPrimitive>(&cmd))
                {
                    auto &drawPrimitiveData = *pDrawPrimitiveData;

                    SimpleMath::Matrix world(*drawPrimitiveData.world);
                    Id::Type meshId = drawPrimitiveData.meshId;
                    Id::Type shaderId = drawPrimitiveData.shaderId;
                    Id::Type texId = drawPrimitiveData.texId;

                    auto meshIt = mMeshMap.find(meshId);
                    if (meshIt != mMeshMap.end())
                    {
                        bool setupDone = false;
                        auto shaderIt = mShaderMap.find(shaderId);
                        if (shaderIt != mShaderMap.end())
                        {
                            BasicEffect *basicEffect = static_cast<BasicEffect *>(shaderIt->second.mEffect.get());
                            auto texIt = mTextureMap.find(texId);
                            if (texIt != mTextureMap.end())
                            {
                                basicEffect->SetTexture(mResourceDescriptors->GetGpuHandle(texIt->second.mIndex),
                                    mStates->AnisotropicWrap());
                                setupDone = true;
                            }

                            basicEffect->SetWorld(world);
                            basicEffect->Apply(commandList);
                        }

                        L_ASSERT(setupDone);
                        meshIt->second.mShape->Draw(commandList);
                    }
                }
            }
            mRenderCommands.clear();
        }
        PIXEndEvent(commandList);

#ifdef EDITOR
        // =================================================================================
        // PASS 2: Render UI / Composite to Back Buffer
        // =================================================================================
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render UI");
        {
            // transition scene texture to SRV so ImGui can read it
            TransitionResource(commandList, targetResource, mSceneState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mSceneState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

            // transition Depth Target for ImGui reading
            TransitionResource(commandList, stencilResource, mSceneDepthState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mSceneDepthState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

            // switch to the actual back buffer for ImGui
            auto backBufferRtv = mDeviceResources->GetRenderTargetView();
            commandList->OMSetRenderTargets(1, &backBufferRtv, FALSE, nullptr);

            // currently clearing to black
            commandList->ClearRenderTargetView(backBufferRtv, Colors::Black, 0, nullptr);

            // render editor
            ImGui::Render();
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

            // update and render additional platorm windows
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }
        PIXEndEvent(commandList);
#endif

        // show the new frame
        PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
        mDeviceResources->Present();
        mGraphicsMemory->Commit(mDeviceResources->GetCommandQueue());
        PIXEndEvent();
    }
#pragma endregion

#pragma region Message Handlers
    void EngineWindowsNT10::OnActivated()
    {
        // TODO: game is becoming active window
    }

    void EngineWindowsNT10::OnDeactivated()
    {
        // TODO: game is becoming background window
    }

    void EngineWindowsNT10::OnSuspending()
    {
        // TODO: game is being power-suspended (or minimized)
    }

    void EngineWindowsNT10::OnResuming()
    {
        mTimer.ResetElapsedTime();

        // TODO: game is being power-resumed (or returning from minimize)
    }

    void EngineWindowsNT10::OnWindowMoved()
    {
        auto r = mDeviceResources->GetOutputSize();
        mDeviceResources->WindowSizeChanged(r.right, r.bottom);
    }

    void EngineWindowsNT10::OnDisplayChange()
    {
        mDeviceResources->UpdateColorSpace();
    }

    void EngineWindowsNT10::OnWindowSizeChanged(int width, int height)
    {
        if (mDeviceResources->GetWindow() != nullptr)
        {
            if (!mDeviceResources->WindowSizeChanged(width, height))
                return;

            CreateWindowSizeDependentResources();

            // TODO: game window is being resized
        }
    }

#ifdef EDITOR
    /// get layout settings
    Engine::LayoutSettings EngineWindowsNT10::GetLayoutSettings() noexcept
    {
        // update cached layout window settings
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if (GetWindowPlacement(mWindow, &wp))
        {
            mLayoutSettings.posX = wp.rcNormalPosition.left;
            mLayoutSettings.posY = wp.rcNormalPosition.top;
            mLayoutSettings.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
            mLayoutSettings.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
            mLayoutSettings.isMaximized = (wp.showCmd == SW_SHOWMAXIMIZED);
        }

        // update cached layout ImGui settings
        if (ImGui::GetCurrentContext())
        {
            std::string line;
            std::vector<std::string> imGuiIniSettings;
            std::stringstream imGuiIniSettingsString(ImGui::SaveIniSettingsToMemory());
            while (std::getline(imGuiIniSettingsString, line))
            {
                imGuiIniSettings.push_back(line);
            }
            if (!imGuiIniSettings.empty())
            {
                mLayoutSettings.imGuiIni = imGuiIniSettings;
            }
        }

        return mLayoutSettings;
    }

    /// set layout settings
    void EngineWindowsNT10::SetLayoutSettings(Engine::LayoutSettings &layoutSettings) noexcept
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);

        // get current placement first to preserve flags we aren't changing
        GetWindowPlacement(GetActiveWindow(), &wp);

        // set cached layout window settings
        wp.rcNormalPosition.left = layoutSettings.posX;
        wp.rcNormalPosition.top = layoutSettings.posY;
        wp.rcNormalPosition.right = layoutSettings.posX + layoutSettings.width;
        wp.rcNormalPosition.bottom = layoutSettings.posY + layoutSettings.height;
        wp.showCmd = layoutSettings.isMaximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
        SetWindowPlacement(GetActiveWindow(), &wp);

        // update the layout cache
        mLayoutSettings = layoutSettings;
    }
#endif

    /// get fullscreen size
    void EngineWindowsNT10::GetFullscreenSize(int &width, int &height) const noexcept
    {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }
#pragma endregion

    /// begin scene
    void EngineWindowsNT10::BeginScene()
    {
    }

    /// push render command
    void EngineWindowsNT10::PushRenderCommand(Engine::RenderCommand renderCommand)
    {
        mRenderCommands.push_back(std::move(renderCommand));
    }

    /// end scene
    void EngineWindowsNT10::EndScene()
    {
    }

    /// create a texture
    Id::Type EngineWindowsNT10::CreateTexture(const TexturePtr &texture, int width, int height)
    {
        Id::Type texId = mTexIdGenerator.Next();
        TextureData textureData;
        textureData.mTexture = texture;
        textureData.mWidth = width;
        textureData.mHeight = height;
        mNewDeviceTextureMap.push_back(mTextureMap.insert_or_assign(texId, std::move(textureData)).first);
        return texId;
    }

    /// release a texture
    void EngineWindowsNT10::ReleaseTexture(Id::Type texId)
    {
        auto it = mTextureMap.find(texId);
        if (it != mTextureMap.end())
        {
            mResourceDescriptors->Free(it->second.mIndex);
            mTextureMap.erase(it);
        }
    }

    /// create a shader
    Id::Type EngineWindowsNT10::CreateShader(const ShaderPtr &shader)
    {
        Id::Type shaderID = mShaderIdGenerator.Next();
        ShaderData shaderData;
        shaderData.mShader = shader;
        shaderData.mEffect;
        auto it = mShaderMap.insert_or_assign(shaderID, std::move(shaderData)).first;
        mNewDeviceShaderMap.push_back(it);
        mNewWindowShaderMap.push_back(it);
        return shaderID;
    }

    /// release a shader
    void EngineWindowsNT10::ReleaseShader(Id::Type shaderID)
    {
        auto it = mShaderMap.find(shaderID);
        if (it != mShaderMap.end())
        {
            mShaderMap.erase(it);
        }
    }

    /// create a mesh
    Id::Type EngineWindowsNT10::CreateMesh(const MeshPtr &mesh)
    {
        Id::Type meshId = mMeshIdGenerator.Next();
        MeshData meshData;
        meshData.mMesh = mesh;
        mNewDeviceMeshMap.push_back(mMeshMap.insert_or_assign(meshId, std::move(meshData)).first);
        return meshId;
    }

    /// release a mesh
    void EngineWindowsNT10::ReleaseMesh(Id::Type meshId)
    {
        auto it = mMeshMap.find(meshId);
        if (it != mMeshMap.end())
        {
            mMeshMap.erase(it);
        }
    }

    /// set render texture size
    void EngineWindowsNT10::SetRenderTextureSize(Id::Type texId, Math::Int2 size)
    {
        //FIXME TESTING ONLY
        // prevent zero sized textures when window is collapsed
        if (size.x > 0 && size.y > 0 &&  (mSceneWidth != size.x || mSceneHeight != size.y))
        {
            mSceneWidth = size.x;
            mSceneHeight = size.y;
            mSceneNeedsResize = true;
        }
        //FIXME TESTING ONLY
    }

    /// get render texture id
    qword EngineWindowsNT10::GetRenderTextureHandle(Id::Type texId)
    {
        //FIXME TESTING ONLY
        if (texId == 0)
        {
            return (qword)mResourceDescriptors->GetGpuHandle(mSceneSRVIndex).ptr;
        }
        else
        {
            return (qword)mResourceDescriptors->GetGpuHandle(mSceneDSVIndex).ptr;
        }
        //FIXME TESTING ONLY
        return (qword)nullptr;
    }

#pragma region Direct3D Resources
    void EngineWindowsNT10::CreateNewDeviceDependentResources()
    {
        auto device = mDeviceResources->GetD3DDevice();

        for (auto &meshDataIt : mNewDeviceMeshMap)
        {
            meshDataIt->second.mShape = GeometricPrimitive::CreateSphere();
#if 0
            meshDataIt->second.mShape = GeometricPrimitive::CreateTorus();
#endif
        }

        ResourceUploadBatch resourceUpload(device);

        resourceUpload.Begin();

        // load textures
        for (auto &textureDataIt : mNewDeviceTextureMap)
        {
            static constexpr int ddsPrefix = sizeof(DWORD) + sizeof(DDS_HEADER);
            static constexpr int elements = 4;
            static constexpr int BPElem = 8;
            int width = textureDataIt->second.mWidth;
            int height = textureDataIt->second.mHeight;
            int surfacePitch = (width * elements * BPElem + (BPElem - 1)) / BPElem;

            std::vector<byte> ddsTexture(ddsPrefix + width * height * elements);
            *((DWORD *)ddsTexture.data()) = DDS_MAGIC;
            DDS_HEADER *header = (DDS_HEADER *)(ddsTexture.data() + sizeof(DWORD));

            memset(header, 0, sizeof(DDS_HEADER));
            header->size = sizeof(DDS_HEADER);
            header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_PITCH | DDS_HEADER_FLAGS_MIPMAP;
            header->height = height;
            header->width = width;
            header->pitchOrLinearSize = surfacePitch;
            header->mipMapCount = 1;
            header->ddspf = DDSPF_A8B8G8R8;
            header->caps = DDS_SURFACE_FLAGS_TEXTURE;

            // create texture
            UniqueByteArray textureData = textureDataIt->second.mTexture->PopTextureData();
            int textureDataPitch = width * elements;
            for (int y = 0; y < height; ++y)
            {
                memcpy(ddsTexture.data() + ddsPrefix + y * surfacePitch, textureData.data() + y * textureDataPitch, textureDataPitch);
            }
            textureDataIt->second.mIndex = mResourceDescriptors->Allocate();
            ThrowIfFailed(
                CreateDDSTextureFromMemory(device, resourceUpload, ddsTexture.data(), ddsTexture.size(), textureDataIt->second.mResource.ReleaseAndGetAddressOf(), true));

            CreateShaderResourceView(device, textureDataIt->second.mResource.Get(), mResourceDescriptors->GetCpuHandle(textureDataIt->second.mIndex));
        }

#if 1
        for (auto &meshDataIt : mNewDeviceMeshMap)
        {
            meshDataIt->second.mShape->LoadStaticBuffers(device, resourceUpload);
        }
#endif

        auto uploadResourcesFinished = resourceUpload.End(
            mDeviceResources->GetCommandQueue());

        uploadResourcesFinished.wait();

        // NOTE: The pipeline state must match the render target format!
        // we are now rendering to our custom scene texture (mSceneRenderTarget),
        // which matches the BackBufferFormat
        RenderTargetState rtState(mDeviceResources->GetBackBufferFormat(),
            mDeviceResources->GetDepthBufferFormat());

        EffectPipelineStateDescription pd(
            &GeometricPrimitive::VertexType::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullNone,
            rtState);

        for (auto &shaderDataIt : mNewDeviceShaderMap)
        {
            //shaderDataIt->second.mEffect = std::make_unique<BasicEffect>(device, EffectFlags::Lighting, pd);
            //shaderDataIt->second.mEffect = std::make_unique<BasicEffect>(device, EffectFlags::Lighting | EffectFlags::Texture, pd);

            bool nIsSimpleDiffuse = shaderDataIt->second.mShader->Name() == "Simple/Diffuse"; //@REVIEW@ FIXME: detected shader

            shaderDataIt->second.mEffect = std::make_unique<BasicEffect>(device, EffectFlags::PerPixelLighting | EffectFlags::Texture, pd);
            BasicEffect *basicEffect = static_cast<BasicEffect *>(shaderDataIt->second.mEffect.get());
            basicEffect->EnableDefaultLighting();
            //basicEffect->SetLightEnabled(0, true);
            //basicEffect->SetLightDiffuseColor(0, Colors::White);
            //basicEffect->SetLightDirection(0, -Vector3::UnitZ);
        }

        mNewDeviceMeshMap.clear();
        mNewDeviceTextureMap.clear();
        mNewDeviceShaderMap.clear();
    }

    void EngineWindowsNT10::CreateNewWindowSizeDependentResources()
    {
        // iterate over all shaders in the map
        for (auto &pair : mShaderMap)
        {
            // get the ShaderData struct
            auto &shaderData = pair.second;

            if (shaderData.mEffect)
            {
                BasicEffect *basicEffect = static_cast<BasicEffect *>(shaderData.mEffect.get());

                // apply the new camera matrices to the effect
                basicEffect->SetView(mView);
                basicEffect->SetProjection(mProj);
            }
        }

        // clear the "New" list because we just updated everything anyway, @@REVIEW@@ we might need to rethink this logic later
        mNewWindowShaderMap.clear();
    }

    /// these are the resources that depend on the device.
    void EngineWindowsNT10::CreateDeviceDependentResources()
    {
        auto device = mDeviceResources->GetD3DDevice();

        // check shader model 6 support
        D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
            || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
        {
#ifdef _DEBUG
            DebugLog::Error("Shader Model 6.0 is not supported!");
#endif
            throw std::runtime_error("Shader Model 6.0 is not supported!");
        }

        // TODO: initialize device dependent objects here (independent of window size)
        mGraphicsMemory = std::make_unique<GraphicsMemory>(device);

        mStates = std::make_unique<CommonStates>(device);

        mResourceDescriptors = std::make_unique<DynamicDescriptorHeap>(device, 256);

        // create descriptor heap for scene render target (RTV)
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 1;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mSceneRTVHeap.ReleaseAndGetAddressOf())));
        mSceneState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        // create descriptor heap for depth stencil target (DSV)
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mSceneDSVHeap.ReleaseAndGetAddressOf())));
        mSceneDepthState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

        CreateNewDeviceDependentResources();
    }

    /// allocate all memory resources that change on a window SizeChanged event
    void EngineWindowsNT10::CreateWindowSizeDependentResources()
    {
        RECT size;
        // TODO: initialize windows-size dependent objects here
#ifdef EDITOR
        // Override size for Editor Mode
        if (mSceneWidth > 0 || mSceneHeight > 0)
        {
            size.left = 0;
            size.right = mSceneWidth;
            size.top = 0;
            size.bottom = mSceneHeight;
        }
        else
        {
            size = mDeviceResources->GetOutputSize();
            size.left = 0;
            size.right = size.right - size.left;
            size.top = 0;
            size.bottom = size.bottom - size.top;
        }
#else
        size = mDeviceResources->GetOutputSize();
#endif

        mView = Matrix::CreateLookAt(Vector3(0.f, 1.f, -2.f),
            Vector3::Zero, Vector3::UnitY);
        mProj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
            float(size.right) / float(size.bottom), 0.1f, 10.f);

#ifdef EDITOR
        // create/resize scene render target texture

        // release previous resource
        mSceneRenderTarget.Reset();
        mSceneDepthStencil.Reset();

        // Ensure SRV descriptor is allocated
        if (mSceneSRVIndex == DynamicDescriptorHeap::InvalidIndex)
        {
            mSceneSRVIndex = mResourceDescriptors->Allocate();
        }

        // Ensure DSV descriptor is allocated
        if (mSceneDSVIndex == DynamicDescriptorHeap::InvalidIndex)
        {
            mSceneDSVIndex = mResourceDescriptors->Allocate();
        }

        // define render target desc
        D3D12_RESOURCE_DESC rtDesc = {};
        rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rtDesc.Alignment = 0;
        rtDesc.Width = size.right;
        rtDesc.Height = size.bottom;
        rtDesc.DepthOrArraySize = 1;
        rtDesc.MipLevels = 1;
        rtDesc.Format = mDeviceResources->GetBackBufferFormat(); // Matches SwapChain
        rtDesc.SampleDesc.Count = 1;
        rtDesc.SampleDesc.Quality = 0;
        rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        // define depth stencil desc
        D3D12_RESOURCE_DESC dsDesc = {};
        dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        dsDesc.Alignment = 0;
        dsDesc.Width = size.right;
        dsDesc.Height = size.bottom;
        dsDesc.DepthOrArraySize = 1;
        dsDesc.MipLevels = 1;
        dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        dsDesc.SampleDesc.Count = 1;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // define optimized clear value
        D3D12_CLEAR_VALUE rtClearValue = {};
        rtClearValue.Format = mDeviceResources->GetBackBufferFormat();
        memcpy(rtClearValue.Color, Colors::CornflowerBlue, sizeof(float) * 4);
        D3D12_CLEAR_VALUE dsClearValue = {};
        dsClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        dsClearValue.DepthStencil.Depth = 1.0f;
        dsClearValue.DepthStencil.Stencil = 0;

        // create committed resource
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        auto device = mDeviceResources->GetD3DDevice();

        // start in SRV state so it is ready for ImGui in first frame (or transitioned before draw)
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &rtDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &rtClearValue,
            IID_PPV_ARGS(mSceneRenderTarget.ReleaseAndGetAddressOf())
        ));
        mSceneRenderTarget->SetName(L"SceneRenderTarget");
        mSceneState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &dsDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &dsClearValue,
            IID_PPV_ARGS(mSceneDepthStencil.ReleaseAndGetAddressOf())
        ));
        mSceneDepthStencil->SetName(L"SceneDepthStencil");
        mSceneDepthState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

        // create RTV in our custom heap
        device->CreateRenderTargetView(mSceneRenderTarget.Get(), nullptr, mSceneRTVHeap->GetCPUDescriptorHandleForHeapStart());

        // create SRV in the main descriptor heap (for ImGui/Shaders)
        device->CreateShaderResourceView(mSceneRenderTarget.Get(), nullptr, mResourceDescriptors->GetCpuHandle(mSceneSRVIndex));

        // create DSV in our custom heap
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // Cast to Depth
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        device->CreateDepthStencilView(mSceneDepthStencil.Get(), &dsvDesc, mSceneDSVHeap->GetCPUDescriptorHandleForHeapStart());

        // create DSV in the main descriptor heap (for ImGui/Shaders)
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Cast to Float for the shader
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        device->CreateShaderResourceView(mSceneDepthStencil.Get(), &srvDesc, mResourceDescriptors->GetCpuHandle(mSceneDSVIndex));
#endif

        CreateNewWindowSizeDependentResources();
    }

    void EngineWindowsNT10::OnDeviceLost()
    {
        // TODO: add Direct3D resource cleanup here
        if (mSceneSRVIndex != DynamicDescriptorHeap::InvalidIndex)
        {
            mResourceDescriptors->Free(mSceneSRVIndex);
            mSceneSRVIndex = DynamicDescriptorHeap::InvalidIndex;
        }
        if (mSceneDSVIndex != DynamicDescriptorHeap::InvalidIndex)
        {
            mResourceDescriptors->Free(mSceneDSVIndex);
            mSceneDSVIndex = DynamicDescriptorHeap::InvalidIndex;
        }
        mSceneRenderTarget.Reset();
        mSceneDepthStencil.Reset();
        mSceneRTVHeap.Reset();
        mSceneDSVHeap.Reset();

        mNewDeviceMeshMap.clear();
        mNewDeviceShaderMap.clear();
        mNewWindowShaderMap.clear();
        mNewDeviceTextureMap.clear();
        for (auto &textureData : std::views::values(mTextureMap))
        {
            textureData.mResource.Reset();
        }
        for (auto &shaderData : std::views::values(mShaderMap))
        {
            shaderData.mEffect.reset();
        }
        for (auto &meshData : std::views::values(mMeshMap))
        {
            meshData.mShape.reset();
        }
        mResourceDescriptors.reset();
        mStates.reset();
        mGraphicsMemory.reset();
    }

    void EngineWindowsNT10::OnDeviceRestored()
    {
        CreateDeviceDependentResources();
        CreateWindowSizeDependentResources();
    }
#pragma endregion

#ifdef EDITOR
    BYTE EngineWindowsNT10::sBuffer[65536];
    OVERLAPPED EngineWindowsNT10::sOverlapped;
    HANDLE EngineWindowsNT10::sDirHandle;
#endif
}

/// Lumen Windows namespace
namespace Lumen::Windows
{
    /// windows procedure
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        static bool s_in_sizemove = false;
        static bool s_in_suspend = false;
        static bool s_minimized = false;
#ifdef _DEBUG
        static bool s_fullscreen = false;
#else
        static bool s_fullscreen = true;
#endif

        Engine *engine = reinterpret_cast<Engine *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        WindowsNT10::EngineWindowsNT10 *engineImpl = nullptr;
        if (engine)
        {
            engineImpl = static_cast<WindowsNT10::EngineWindowsNT10 *>(engine->GetImpl());
        }

#ifdef EDITOR
        if (engineImpl && engineImpl->ImGuiInitialized())
        {
            ImGuiIO &io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureMouseUnlessPopupClose)
            {
                switch (message)
                {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_RBUTTONDBLCLK:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MBUTTONDBLCLK:
                case WM_MOUSEWHEEL:
                case WM_MOUSEMOVE:
                    ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
                    return TRUE;
                }
            }
            if (io.WantCaptureKeyboard)
            {
                switch (message)
                {
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_CHAR:
                    ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
                    return TRUE;
                }
            }
            if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            {
                return TRUE;
            }
        }
#endif

        switch (message)
        {
        case WM_PAINT:
            if (s_in_sizemove && engine)
            {
                engine->Run();
            }
            else
            {
                PAINTSTRUCT ps;
                (void)BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_DISPLAYCHANGE:
            if (engineImpl)
            {
                engineImpl->OnDisplayChange();
            }
            break;

        case WM_MOVE:
            if (engineImpl)
            {
                engineImpl->OnWindowMoved();
            }
            break;

        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                if (!s_minimized)
                {
                    s_minimized = true;
                    if (!s_in_suspend && engineImpl)
                        engineImpl->OnSuspending();
                    s_in_suspend = true;
                }
            }
            else if (s_minimized)
            {
                s_minimized = false;
                if (s_in_suspend && engineImpl)
                    engineImpl->OnResuming();
                s_in_suspend = false;
            }
            else if (!s_in_sizemove && engineImpl)
            {
                engineImpl->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            }
            break;

        case WM_ENTERSIZEMOVE:
            s_in_sizemove = true;
            break;

        case WM_EXITSIZEMOVE:
            s_in_sizemove = false;
            if (engineImpl)
            {
                RECT rc;
                GetClientRect(hWnd, &rc);

                engineImpl->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            }
            break;

        case WM_GETMINMAXINFO:
            if (lParam)
            {
                auto info = reinterpret_cast<MINMAXINFO *>(lParam);
                info->ptMinTrackSize.x = 320;
                info->ptMinTrackSize.y = 200;
            }
            break;

        case WM_ACTIVATEAPP:
            if (engineImpl)
            {
                if (wParam)
                {
                    engineImpl->OnActivated();
                }
                else
                {
                    engineImpl->OnDeactivated();
                }
            }
            break;

        case WM_POWERBROADCAST:
            switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
                if (!s_in_suspend && engineImpl)
                    engineImpl->OnSuspending();
                s_in_suspend = true;
                return TRUE;

            case PBT_APMRESUMESUSPEND:
                if (!s_minimized)
                {
                    if (s_in_suspend && engineImpl)
                        engineImpl->OnResuming();
                    s_in_suspend = false;
                }
                return TRUE;
            }
            break;

#ifdef EDITOR
        case WM_CLOSE:
            if (engineImpl)
            {
                engineImpl->GetLayoutSettings();
            }
            break;
#endif

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

#if defined(_DEBUG) && !defined(EDITOR)
        case WM_SYSKEYDOWN:
            if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
            {
                // implements the classic ALT+ENTER fullscreen toggle
                if (s_fullscreen)
                {
                    SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                    int width = 1280 / 3;
                    int height = 720 / 3;
                    if (auto application = engine->GetApplication().lock())
                    {
                        application->GetWindowSize(width, height);
                    }

                    ShowWindow(hWnd, SW_SHOWNORMAL);
                    SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }
                else
                {
                    SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                    SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
                }

                s_fullscreen = !s_fullscreen;
            }
            break;
#endif

        case WM_MENUCHAR:
            // a menu is active and the user presses a key that does not correspond
            // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
            return MAKELRESULT(0, MNC_CLOSE);
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

//==============================================================================================================================================================================

/// allocate smart pointer version of the engine, implemented at platform level
EnginePtr Engine::MakePtr(const ApplicationPtr &application)
{
    auto ptr = EnginePtr(new Engine(application, new WindowsNT10::EngineWindowsNT10()));
    application->SetEngine(ptr);
    ptr->mImpl->SetOwner(ptr);
    return ptr;
}

/// debug log, implemented at platform level
void Engine::DebugOutput(const std::string &message)
{
    OutputDebugStringA((message+'\n').c_str());
}
