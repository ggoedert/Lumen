//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindowsNT10.h"
#include "lFolderFileSystem.h"
#include "lEngine.h"

// helpers headers
#include "EngineFramework.h"
#include "DeviceResources.h"
#include "StepTimer.h"
#include "DDS.h"

using namespace DX;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Lumen;

using Microsoft::WRL::ComPtr;

/// Lumen WindowsNT10 namespace
namespace Lumen::WindowsNT10
{
    /// Engine windows NT10 implementation.
    /// creates a D3D12 device and provides a game loop
    class EngineWindowsNT10 final : public Engine, public DX::IDeviceNotify
    {
        CLASS_NO_DEFAULT_CTOR(EngineWindowsNT10);
        CLASS_NO_COPY_MOVE(EngineWindowsNT10);

    public:
        EngineWindowsNT10(const ApplicationPtr &application) noexcept(false);
        ~EngineWindowsNT10();

        // initialization and management
        bool Initialize(const Object &config) override;

        // basic game loop
        bool Tick() override;

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

    private:
        bool Update(StepTimer const &timer);
        void Render();

        void Clear();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

        // device resources
        std::unique_ptr<DX::DeviceResources> mDeviceResources;

        // rendering loop timer
        StepTimer mTimer;

        std::unique_ptr<DirectX::GraphicsMemory> mGraphicsMemory;
        std::unique_ptr<DirectX::CommonStates> mStates;

        DirectX::SimpleMath::Matrix mWorld;
        DirectX::SimpleMath::Matrix mView;
        DirectX::SimpleMath::Matrix mProj;

        std::unique_ptr<DirectX::GeometricPrimitive> mShape;

        std::unique_ptr<DirectX::BasicEffect> mEffect;

        std::unique_ptr<DirectX::DescriptorHeap> mResourceDescriptors;
        Microsoft::WRL::ComPtr<ID3D12Resource> mTexture;

        enum Descriptors : size_t
        {
            Procedural,
            Count
        };
    };

    EngineWindowsNT10::EngineWindowsNT10(const ApplicationPtr &application) noexcept(false) :
        mDeviceResources(std::make_unique<DeviceResources>()),
        Engine(application)
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
        if (config.Type() != Lumen::WindowsNT10::Config::Type())
        {
#ifdef TYPEINFO
            DebugLog::Error("Initialize engine, unknown config type: {}", config.Type().mName);
#else
            DebugLog::Error("Initialize engine, unknown config hash type: 0x{:08X}", config.Type());
#endif
            return false;
        }

        const auto &initializeConfig = static_cast<const Lumen::WindowsNT10::Config &>(config);
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

        // initialize base
        return Engine::Initialize(config);
    }

#pragma region Frame Update
    /// executes the basic game loop
    bool EngineWindowsNT10::Tick()
    {
        bool updateResult = true;

        Engine::Run(static_cast<float>(mTimer.GetElapsedSeconds()));

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

        mResourceDescriptors = std::make_unique<DescriptorHeap>(device,
            Descriptors::Count);

        mShape = GeometricPrimitive::CreateSphere();

#if 0
        mShape = GeometricPrimitive::CreateTorus();
#endif

        ResourceUploadBatch resourceUpload(device);

        resourceUpload.Begin();

#define DDS_PREFIX (sizeof(DWORD) + sizeof(DirectX::DDS_HEADER))
#define WIDTH    64
#define HEIGHT   64
#define ELEMENTS  4
#define B_P_ELEM  8
#define SCREEN_TEX_PITCH ((WIDTH * ELEMENTS * B_P_ELEM + (B_P_ELEM - 1)) / B_P_ELEM)

        std::vector<byte> m_ddsTexture(DDS_PREFIX + WIDTH * HEIGHT * ELEMENTS);
        *((DWORD *)m_ddsTexture.data()) = DDS_MAGIC;
        DDS_HEADER *header = (DDS_HEADER *)(m_ddsTexture.data() + sizeof(DWORD));

        memset(header, 0, sizeof(DDS_HEADER));
        header->size = sizeof(DDS_HEADER);
        header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_PITCH | DDS_HEADER_FLAGS_MIPMAP;
        header->height = HEIGHT;
        header->width = WIDTH;
        header->pitchOrLinearSize = SCREEN_TEX_PITCH;
        header->mipMapCount = 1;
        header->ddspf = DDSPF_A8B8G8R8;
        header->caps = DDS_SURFACE_FLAGS_TEXTURE;

        byte *tex = m_ddsTexture.data() + DDS_PREFIX;
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                if ((x < (WIDTH >> 1)) == (y < (HEIGHT >> 1)))
                {
                    tex[y * SCREEN_TEX_PITCH + ELEMENTS * x + 0] = 198;
                    tex[y * SCREEN_TEX_PITCH + ELEMENTS * x + 1] = 197;
                    tex[y * SCREEN_TEX_PITCH + ELEMENTS * x + 2] = 198;
                }
                else
                {
                    tex[y * SCREEN_TEX_PITCH + ELEMENTS * x + 0] = 156;
                    tex[y * SCREEN_TEX_PITCH + ELEMENTS * x + 1] = 158;
                    tex[y * SCREEN_TEX_PITCH + ELEMENTS * x + 2] = 156;
                }
                tex[y * SCREEN_TEX_PITCH + ELEMENTS * x + 3] = 255;
            }
        }

        ThrowIfFailed(
            CreateDDSTextureFromMemory(device, resourceUpload, m_ddsTexture.data(), m_ddsTexture.size(), mTexture.ReleaseAndGetAddressOf(), true));

        CreateShaderResourceView(device, mTexture.Get(),
            mResourceDescriptors->GetCpuHandle(Descriptors::Procedural));

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

        mEffect->SetTexture(mResourceDescriptors->GetGpuHandle(Descriptors::Procedural),
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
        mTexture.Reset();
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
    return std::make_shared<WindowsNT10::EngineWindowsNT10>(application);
}

/// debug log, implemented at platform level
void Engine::DebugOutput(const std::string &message)
{
    OutputDebugStringA((message+'\n').c_str());
}
