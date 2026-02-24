//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows dx12 implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindows.h"
#include "lFolderFileSystem.h"
#include "lTexture.h"
#include "lShader.h"
#include "lMesh.h"
#include "lEngine.h"
#include "lDrawPrimitive.h"

#include "EngineWindows.h"

// helpers headers
#include "EngineFramework.h"
#include "DeviceResources.h"
#include "DynamicDescriptorHeap.h"
#include "StepTimer.h"
#include "DDS.h"

#ifdef EDITOR
// ImGui editor support
#include "lImGuiLibDX12.h"
#endif

using namespace DX;
using namespace DirectX;
using namespace SimpleMath;
using namespace Lumen;

using Microsoft::WRL::ComPtr;

/// Lumen Windows::NT10 namespace
namespace Lumen::Windows::NT10
{
    /// Engine windows DX12 implementation
    class EngineDX12 : public Windows::EngineWindows, public DX::IDeviceNotify
    {
        CLASS_NO_COPY_MOVE(EngineDX12);

    public:
        /// constructs an engine
        explicit EngineDX12();

        /// destroys engine
        ~EngineDX12() override;

        /// initialization and management
        bool Initialize(const Object &config) override;

#ifdef EDITOR
        /// check if initialized
        bool Initialized() override { return mInitialized; }
#endif

        /// create new resources
        bool CreateNewResources() override;

        /// shutdown
        void Shutdown() override;

        // basic game loop
        bool Run(std::function<bool()> update, std::function<void()> preRender) override;

        // get elapsed time since last run
        float GetElapsedTime() override;

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

        // get fullscreen size
        void GetFullscreenSize(int &width, int &height) const noexcept override;

        /// create a file system for the assets
        IFileSystemPtr AssetsFileSystem() const override
        {
            return FolderFileSystem::MakePtr("Assets");
        }

        /// post event
        void PostEvent(EventUniquePtr event) override;

        /// post render command
        void PostRenderCommand(RenderCommandUniquePtr renderCommand) override;

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

    private:
        void Render();

        void CreateNewDeviceDependentResources();
        void CreateNewWindowSizeDependentResources();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

#ifdef EDITOR
        /// initialized
        bool mInitialized = false;

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

        std::vector<RenderCommandUniquePtr> mRenderCommands;
    };

    /// constructs an engine
    EngineDX12::EngineDX12() : Windows::EngineWindows(), mDeviceResources(std::make_unique<DeviceResources>())
    {
        // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
        //   Add DeviceResources::c_AllowTearing to opt-in to variable rate displays.
        //   Add DeviceResources::c_EnableHDR for HDR10 display.
        mDeviceResources->RegisterDeviceNotify(this);
    }

    /// destroys engine
    EngineDX12::~EngineDX12()
    {
        if (mDeviceResources)
        {
            mDeviceResources->WaitForGpu();
        }
    }

    /// initialize the Direct3D resources required to run
    bool EngineDX12::Initialize(const Object &config)
    {
#ifdef EDITOR
        // make sure we have the required assets for the editor
        if (!std::filesystem::exists(Lumen::ImGuiLib::gMaterialIconsFontFilename))
        {
            Lumen::DebugLog::Error("EngineDX12::Initialize font file does not exist, {}", Lumen::ImGuiLib::gMaterialIconsFontFilename);
            return false;
        }

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
#ifdef EDITOR
        /// get cached settings
        Engine::Settings settings = GetSettings();
        mSceneWidth = settings.width;
        mSceneHeight = settings.height;
#else
        RECT rc;
        GetClientRect(initializeConfig.mWindow, &rc);
        OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        mSceneWidth = rc.right - rc.left;
        mSceneHeight = rc.bottom - rc.top;
#endif
        mSceneNeedsResize = false;
        mDeviceResources->SetWindow(initializeConfig.mWindow, mSceneWidth, mSceneHeight);

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
        for (const auto &line : settings.imGuiIni)
        {
            combinedImGuiIni += line;
            combinedImGuiIni += '\n';
        }
        ImGui::LoadIniSettingsFromMemory(combinedImGuiIni.c_str(), combinedImGuiIni.size());

        // setup scaling
        ImGuiStyle &style = ImGui::GetStyle();
        style.ScaleAllSizes(mMainScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = mMainScale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
        io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
        io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

        // when viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(initializeConfig.mWindow);

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

        // Merge Material Icons PUA range
        io.Fonts->AddFontDefault(); // ensures a base font exists
        ImFontConfig imFontConfig;
        imFontConfig.MergeMode = true;
        imFontConfig.PixelSnapH = true;
        imFontConfig.GlyphOffset.y = Lumen::ImGuiLib::gMaterialIconsIconsGlyphOffset;
        Lumen::ImGuiLib::gMaterialIconsFont = io.Fonts->AddFontFromFileTTF(Lumen::ImGuiLib::gMaterialIconsFontFilename, Lumen::ImGuiLib::gMaterialIconsFontSize, &imFontConfig,
            Lumen::ImGuiLib::gMaterialIconsIconsRanges);
        ImGui_ImplDX12_CreateDeviceObjects(); // build font atlas

        // mark initialized (after this point, we can start processing file change callbacks and trigger events)
        mInitialized = true;
#endif

        // call base initialize
        if (!EngineWindows::Initialize(config))
        {
            return false;
        }

#ifdef EDITOR
        // trigger initial window size changed to setup ImGui
        RECT rc;
        GetClientRect(initializeConfig.mWindow, &rc);
        OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
#endif

        return true;
    }

    /// create resources
    bool EngineDX12::CreateNewResources()
    {
        CreateNewDeviceDependentResources();
        CreateNewWindowSizeDependentResources();
        return true;
    }

    /// shutdown
    void EngineDX12::Shutdown()
    {
        mDeviceResources->WaitForGpu();

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

#pragma region Frame Update
    /// executes the basic game loop
    bool EngineDX12::Run(std::function<bool()> update, std::function<void()> preRender)
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

    // get elapsed time since last run
    float EngineDX12::GetElapsedTime()
    {
        return static_cast<float>(mTimer.GetElapsedSeconds());
    }

#pragma region Frame Render
    /// draws the scene
    void EngineDX12::Render()
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
                if (cmd->Type() == DrawPrimitive::Type())
                {
                    auto &drawPrimitiveData = static_cast<DrawPrimitive &>(*cmd);

                    SimpleMath::Matrix world(*drawPrimitiveData.mWorld);
                    Id::Type meshId = drawPrimitiveData.mMeshId;
                    Id::Type shaderId = drawPrimitiveData.mShaderId;
                    Id::Type texId = drawPrimitiveData.mTexId;

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
    void EngineDX12::OnActivated()
    {
        // TODO: game is becoming active window
    }

    void EngineDX12::OnDeactivated()
    {
        // TODO: game is becoming background window
    }

    void EngineDX12::OnSuspending()
    {
        // TODO: game is being power-suspended (or minimized)
    }

    void EngineDX12::OnResuming()
    {
        mTimer.ResetElapsedTime();

        // TODO: game is being power-resumed (or returning from minimize)
    }

    void EngineDX12::OnWindowMoved()
    {
        auto r = mDeviceResources->GetOutputSize();
        mDeviceResources->WindowSizeChanged(r.right, r.bottom);
    }

    void EngineDX12::OnDisplayChange()
    {
        mDeviceResources->UpdateColorSpace();
    }

    void EngineDX12::OnWindowSizeChanged(int width, int height)
    {
        if (mDeviceResources->GetWindow() != nullptr)
        {
            if (!mDeviceResources->WindowSizeChanged(width, height))
                return;

            CreateWindowSizeDependentResources();

            // TODO: game window is being resized
        }
    }

    /// get fullscreen size
    void EngineDX12::GetFullscreenSize(int &width, int &height) const noexcept
    {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }
#pragma endregion

    /// post event
    void EngineDX12::PostEvent(EventUniquePtr event)
    {
        Hash eventType = event->Type();
        if (eventType == EngineWindows::Activated)
        {
            OnActivated();
        }
        else if (eventType == EngineWindows::Deactivated)
        {
            OnDeactivated();
        }
        else if (eventType == EngineWindows::Suspending)
        {
            OnSuspending();
        }
        else if (eventType == EngineWindows::Resuming)
        {
            OnResuming();
        }
        else if (eventType == EngineWindows::WindowMoved)
        {
            OnWindowMoved();
        }
        if (eventType == EngineWindows::DisplayChanged)
        {
            OnDisplayChange();
        }
        else if (eventType == EngineWindows::WindowSizeChanged::Type())
        {
            auto &windowSizeChangedEvent = static_cast<EngineWindows::WindowSizeChanged &>(*event);
            OnWindowSizeChanged(windowSizeChangedEvent.mWidth, windowSizeChangedEvent.mHeight);
        }
    }

    /// post render command
    void EngineDX12::PostRenderCommand(RenderCommandUniquePtr renderCommand)
    {
        mRenderCommands.push_back(std::move(renderCommand));
    }

    /// create a texture
    Id::Type EngineDX12::CreateTexture(const TexturePtr &texture, int width, int height)
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
    void EngineDX12::ReleaseTexture(Id::Type texId)
    {
        auto it = mTextureMap.find(texId);
        if (it != mTextureMap.end())
        {
            mResourceDescriptors->Free(it->second.mIndex);
            mTextureMap.erase(it);
        }
    }

    /// create a shader
    Id::Type EngineDX12::CreateShader(const ShaderPtr &shader)
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
    void EngineDX12::ReleaseShader(Id::Type shaderID)
    {
        auto it = mShaderMap.find(shaderID);
        if (it != mShaderMap.end())
        {
            mShaderMap.erase(it);
        }
    }

    /// create a mesh
    Id::Type EngineDX12::CreateMesh(const MeshPtr &mesh)
    {
        Id::Type meshId = mMeshIdGenerator.Next();
        MeshData meshData;
        meshData.mMesh = mesh;
        mNewDeviceMeshMap.push_back(mMeshMap.insert_or_assign(meshId, std::move(meshData)).first);
        return meshId;
    }

    /// release a mesh
    void EngineDX12::ReleaseMesh(Id::Type meshId)
    {
        auto it = mMeshMap.find(meshId);
        if (it != mMeshMap.end())
        {
            mMeshMap.erase(it);
        }
    }

    /// set render texture size
    void EngineDX12::SetRenderTextureSize(Id::Type texId, Math::Int2 size)
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
    qword EngineDX12::GetRenderTextureHandle(Id::Type texId)
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
    void EngineDX12::CreateNewDeviceDependentResources()
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

    void EngineDX12::CreateNewWindowSizeDependentResources()
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
    void EngineDX12::CreateDeviceDependentResources()
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
    void EngineDX12::CreateWindowSizeDependentResources()
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

    void EngineDX12::OnDeviceLost()
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

    void EngineDX12::OnDeviceRestored()
    {
        CreateDeviceDependentResources();
        CreateWindowSizeDependentResources();
    }
#pragma endregion
}

/// create a smart pointer version of the engine, dx12 version
EnginePtr Lumen::Windows::CreateEngine(const ApplicationPtr &application)
{
    return Engine::MakePtr(new Lumen::Windows::NT10::EngineDX12(), application);
}
