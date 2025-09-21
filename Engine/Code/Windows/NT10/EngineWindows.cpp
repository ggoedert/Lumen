//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindows.h"
#include "lFolderFileSystem.h"
#include "lTexture.h"
#include "lEngine.h"

#include "EngineImpl.h"

// helpers headers
#include "EngineFramework.h"
#include "DeviceResources.h"
#include "DynamicDescriptorHeap.h"
#include "StepTimer.h"
#include "DDS.h"

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
        ~EngineWindowsNT10();

        // initialization and management
        bool Initialize(const Object &config) override;

        /// shutdown
        void Shutdown() override;

        // get elapsed time since last run
        float GetElapsedTime() override;

        // basic game loop
        bool Run() override;

        // IDeviceNotify
        void OnDeviceLost() override;
        void OnDeviceRestored() override;

        // messages
        void OnActivated() override;
        void OnDeactivated() override;
        void OnSuspending() override;
        void OnResuming() override;
        void OnWindowMoved() override;
        void OnDisplayChange() override;
        void OnWindowSizeChanged(int width, int height) override;

        // properties
        void GetDefaultSize(int &width, int &height) const noexcept override;

        /// create a folder file system
        IFileSystemPtr FolderFileSystem(std::string_view name, std::string_view path) const override
        {
            return FolderFileSystem::MakePtr(name, path);
        }

        /// register a texture
        Engine::TextureID RegisterTexture(const TexturePtr &texture, int width, int height) override;

        /// unregister a texture
        void UnregisterTexture(Engine::TextureID texID) override;

    private:
        /// generate next TextureID
        Engine::TextureID GenerateNextTextureID();

        bool Update(StepTimer const &timer);
        void Render();

        void Clear();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

        // device resources
        std::unique_ptr<DX::DeviceResources> mDeviceResources;

        // rendering loop timer
        StepTimer mTimer;

        std::unique_ptr<GraphicsMemory> mGraphicsMemory;
        std::unique_ptr<CommonStates> mStates;

        SimpleMath::Matrix mWorld;
        SimpleMath::Matrix mView;
        SimpleMath::Matrix mProj;

        std::unique_ptr<GeometricPrimitive> mShape;

        std::unique_ptr<BasicEffect> mEffect;

        std::unique_ptr<DynamicDescriptorHeap> mResourceDescriptors;

        /// next texture id
        Engine::TextureID mNextTextureID;

        /// map of texture
        struct TextureData
        {
            TexturePtr mTexture;
            DynamicDescriptorHeap::IndexType mIndex;
            Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
            int mWidth;
            int mHeight;
        };
        std::unordered_map<Engine::TextureID, TextureData> mTextureMap;
    };

    EngineWindowsNT10::EngineWindowsNT10() :
        mDeviceResources(std::make_unique<DeviceResources>()),
        mNextTextureID(0)
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
        if (config.Type() != Lumen::Windows::Config::Type())
        {
#ifdef TYPEINFO
            DebugLog::Error("Initialize engine, unknown config type: {}", config.Type().mName);
#else
            DebugLog::Error("Initialize engine, unknown config hash type: 0x{:08X}", config.Type());
#endif
            return false;
        }

        const auto &initializeConfig = static_cast<const Lumen::Windows::Config &>(config);
        mDeviceResources->SetWindow(initializeConfig.mWindow, initializeConfig.mWidth, initializeConfig.mHeight);

        mDeviceResources->CreateDeviceResources();
        CreateDeviceDependentResources();

        mDeviceResources->CreateWindowSizeDependentResources();
        CreateWindowSizeDependentResources();

        // TODO: change the timer settings if you want something other than the default variable timestep mode
        // e.g. for 60 FPS fixed timestep update logic, call:
        /*
        mTimer.SetFixedTimeStep(true);
        mTimer.SetTargetElapsedSeconds(1.0 / 60);
        */

        return true;
    }

    /// shutdown
    void EngineWindowsNT10::Shutdown()
    {
        // release textures
        auto keyView = std::views::keys(mTextureMap);
        for (auto &texID : std::vector<Engine::TextureID>(keyView.begin(), keyView.end()))
        {
            auto it = mTextureMap.find(texID);
            if (it != mTextureMap.end())
            {
                it->second.mTexture->Unregister();
            }
        }
        mTextureMap.clear();
    }

    // get elapsed time since last run
    float EngineWindowsNT10::GetElapsedTime()
    {
        return static_cast<float>(mTimer.GetElapsedSeconds());
    }

#pragma region Frame Update
    /// executes the basic game loop
    bool EngineWindowsNT10::Run()
    {
        bool updateResult = true;

        mTimer.Tick([&]()
        {
            if (updateResult)
                updateResult = Update(mTimer);
        });
        if (!updateResult)
            return false;

        Render();
        return true;
    }

    /// updates the world
    bool EngineWindowsNT10::Update(StepTimer const &timer)
    {
        bool runResult = true;

        PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

        auto time = static_cast<float>(timer.GetTotalSeconds());

        // TODO: add your game logic here
#if 0
        mWorld = Matrix::CreateRotationZ(cosf(time) * 2.f);
#endif

#if 1
        mWorld = Matrix::CreateRotationY(time);
#endif

        PIXEndEvent();

        return runResult;
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

        // prepare the command list to render a new frame
        mDeviceResources->Prepare();
        Clear();

        auto commandList = mDeviceResources->GetCommandList();
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

        // TODO: add your rendering code here
        ID3D12DescriptorHeap *heaps[] = { mResourceDescriptors->Heap(), mStates->Heap() };
        commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

        mEffect->SetWorld(mWorld);
        mEffect->Apply(commandList);

        mShape->Draw(commandList);

        PIXEndEvent(commandList);

        // show the new frame
        PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
        mDeviceResources->Present();
        mGraphicsMemory->Commit(mDeviceResources->GetCommandQueue());
        PIXEndEvent();
    }

    /// helper method to clear the back buffers
    void EngineWindowsNT10::Clear()
    {
        auto commandList = mDeviceResources->GetCommandList();
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

        // clear the views
        auto rtvDescriptor = mDeviceResources->GetRenderTargetView();
        auto dsvDescriptor = mDeviceResources->GetDepthStencilView();

        commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
        commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
        commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        // set the viewport and scissor rect
        auto viewport = mDeviceResources->GetScreenViewport();
        auto scissorRect = mDeviceResources->GetScissorRect();
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        PIXEndEvent(commandList);
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
        if (!mDeviceResources->WindowSizeChanged(width, height))
            return;

        CreateWindowSizeDependentResources();

        // TODO: game window is being resized
    }

    // properties
    void EngineWindowsNT10::GetDefaultSize(int &width, int &height) const noexcept
    {
        // TODO: change to desired default window size (note minimum size is 320x200)
        width = 1600;
        height = 900;
    }
#pragma endregion

    /// register a texture
    Engine::TextureID EngineWindowsNT10::RegisterTexture(const TexturePtr &texture, int width, int height)
    {
        Engine::TextureID texID = GenerateNextTextureID();
        TextureData textureData;
        textureData.mTexture = texture;
        textureData.mWidth = width;
        textureData.mHeight = width;
        mTextureMap[texID] = textureData;
        return texID;
    }

    /// unregister a texture
    void EngineWindowsNT10::UnregisterTexture(Engine::TextureID texID)
    {
        auto it = mTextureMap.find(texID);
        if (it != mTextureMap.end())
        {
            mTextureMap.erase(it);
        }
    }

    /// generate next TextureID
    Engine::TextureID EngineWindowsNT10::GenerateNextTextureID()
    {
        return mNextTextureID++;
    }

#pragma region Direct3D Resources
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

        mShape = GeometricPrimitive::CreateSphere();

#if 0
        mShape = GeometricPrimitive::CreateTorus();
#endif

        ResourceUploadBatch resourceUpload(device);

        resourceUpload.Begin();

        // ???
        auto it = mTextureMap.find(0/*mTextureIndex*/);
        if (it != mTextureMap.end())
        {
            static constexpr int ddsPrefix = sizeof(DWORD) + sizeof(DDS_HEADER);
            static constexpr int elements = 4;
            static constexpr int BPElem = 8;
            int width = it->second.mWidth;
            int height = it->second.mHeight;
            int screenTexPitch = (width * elements * BPElem + (BPElem - 1)) / BPElem;

            std::vector<byte> ddsTexture(ddsPrefix + width * height * elements);
            *((DWORD *)ddsTexture.data()) = DDS_MAGIC;
            DDS_HEADER *header = (DDS_HEADER *)(ddsTexture.data() + sizeof(DWORD));

            memset(header, 0, sizeof(DDS_HEADER));
            header->size = sizeof(DDS_HEADER);
            header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_PITCH | DDS_HEADER_FLAGS_MIPMAP;
            header->height = height;
            header->width = width;
            header->pitchOrLinearSize = screenTexPitch;
            header->mipMapCount = 1;
            header->ddspf = DDSPF_A8B8G8R8;
            header->caps = DDS_SURFACE_FLAGS_TEXTURE;

            // ??? fill texture with a pattern
            it->second.mTexture->GetTextureData(ddsTexture.data() + ddsPrefix, screenTexPitch);
            it->second.mIndex = mResourceDescriptors->Allocate();
            ThrowIfFailed(
                CreateDDSTextureFromMemory(device, resourceUpload, ddsTexture.data(), ddsTexture.size(), it->second.mResource.ReleaseAndGetAddressOf(), true));

            CreateShaderResourceView(device, it->second.mResource.Get(), mResourceDescriptors->GetCpuHandle(it->second.mIndex));
        }

#if 1
        mShape->LoadStaticBuffers(device, resourceUpload);
#endif

        auto uploadResourcesFinished = resourceUpload.End(
            mDeviceResources->GetCommandQueue());

        uploadResourcesFinished.wait();

        RenderTargetState rtState(mDeviceResources->GetBackBufferFormat(),
            mDeviceResources->GetDepthBufferFormat());

        EffectPipelineStateDescription pd(
            &GeometricPrimitive::VertexType::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullNone,
            rtState);

        //mEffect = std::make_unique<BasicEffect>(device, EffectFlags::Lighting, pd);
        //mEffect = std::make_unique<BasicEffect>(device, EffectFlags::Lighting | EffectFlags::Texture, pd);
        mEffect = std::make_unique<BasicEffect>(device, EffectFlags::PerPixelLighting | EffectFlags::Texture, pd);
        //mEffect->EnableDefaultLighting();
        mEffect->SetLightEnabled(0, true);
        mEffect->SetLightDiffuseColor(0, Colors::White);
        mEffect->SetLightDirection(0, -Vector3::UnitZ);

        mEffect->SetTexture(mResourceDescriptors->GetGpuHandle(0/*mTextureIndex*/),
            mStates->AnisotropicWrap());

        mWorld = Matrix::Identity;
    }

    /// allocate all memory resources that change on a window SizeChanged event
    void EngineWindowsNT10::CreateWindowSizeDependentResources()
    {
        // TODO: initialize windows-size dependent objects here
        auto size = mDeviceResources->GetOutputSize();

        mView = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
            Vector3::Zero, Vector3::UnitY);
        mProj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
            float(size.right) / float(size.bottom), 0.1f, 10.f);

        mEffect->SetView(mView);
        mEffect->SetProjection(mProj);
    }

    void EngineWindowsNT10::OnDeviceLost()
    {
        // TODO: add Direct3D resource cleanup here
        mShape.reset();
        mEffect.reset();
        for (auto &textureData : std::views::values(mTextureMap))
        {
            textureData.mResource.Reset();
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
}

//==============================================================================================================================================================================

/// allocate smart pointer version of the engine, implemented at platform level
Lumen::EnginePtr Engine::MakePtr(const ApplicationPtr &application)
{
    return EnginePtr(new Lumen::Engine(application, new Lumen::WindowsNT10::EngineWindowsNT10()));
}

/// debug log, implemented at platform level
void Engine::DebugOutput(const std::string &message)
{
    OutputDebugStringA((message+'\n').c_str());
}
