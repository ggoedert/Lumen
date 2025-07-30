//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindows.h"
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

/// Lumen WindowsNT6_1 namespace
namespace Lumen::WindowsNT6_1
{
    /// Engine windows NT6_1 implementation.
    /// creates a D3D11 device and provides a game loop
    class EngineWindowsNT6_1 final : public Engine, public DX::IDeviceNotify
    {
        CLASS_NO_DEFAULT_CTOR(EngineWindowsNT6_1);
        CLASS_NO_COPY_MOVE(EngineWindowsNT6_1);

    public:
        EngineWindowsNT6_1(const ApplicationPtr &application) noexcept(false);

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

        DirectX::SimpleMath::Matrix mWorld;
        DirectX::SimpleMath::Matrix mView;
        DirectX::SimpleMath::Matrix mProj;

        std::unique_ptr<DirectX::GeometricPrimitive> mShape;

        std::unique_ptr<DirectX::BasicEffect> mEffect;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mTexture;

        enum Descriptors : size_t
        {
            Procedural,
            Count
        };
    };

    EngineWindowsNT6_1::EngineWindowsNT6_1(const ApplicationPtr &application) noexcept(false) :
        mDeviceResources(std::make_unique<DeviceResources>()),
        Engine(application)
    {
        // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
        //   Add DeviceResources::c_AllowTearing to opt-in to variable rate displays.
        //   Add DeviceResources::c_EnableHDR for HDR10 display.
        mDeviceResources->RegisterDeviceNotify(this);
    }

    /// initialize the Direct3D resources required to run
    bool EngineWindowsNT6_1::Initialize(const Object &config)
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

        // initialize base
        return Engine::Initialize(config);
    }

#pragma region Frame Update
    /// executes the basic game loop
    bool EngineWindowsNT6_1::Tick()
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
    bool EngineWindowsNT6_1::Update(StepTimer const &timer)
    {
        bool runResult = true;

        auto time = static_cast<float>(timer.GetTotalSeconds());

        // TODO: add your game logic here
#if 0
        mWorld = Matrix::CreateRotationZ(cosf(time) * 2.f);
#endif

#if 1
        mWorld = Matrix::CreateRotationY(time);
#endif

        return runResult;
    }
#pragma endregion

#pragma region Frame Render
    /// draws the scene
    void EngineWindowsNT6_1::Render()
    {
        // don't try to render anything before the first Update
        if (mTimer.GetFrameCount() == 0)
        {
            return;
        }

        Clear();

        mDeviceResources->PIXBeginEvent(L"Render");
        //auto context = mDeviceResources->GetD3DDeviceContext();

        // TODO: add your rendering code here
        mEffect->SetWorld(mWorld);
        mShape->Draw(mEffect.get(), mInputLayout.Get());

        mDeviceResources->PIXEndEvent();

        // show the new frame
        mDeviceResources->Present();
    }

    /// helper method to clear the back buffers
    void EngineWindowsNT6_1::Clear()
    {
        mDeviceResources->PIXBeginEvent(L"Clear");

        // clear the views
        auto context = mDeviceResources->GetD3DDeviceContext();
        auto renderTarget = mDeviceResources->GetRenderTargetView();
        auto depthStencil = mDeviceResources->GetDepthStencilView();

        context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
        context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        context->OMSetRenderTargets(1, &renderTarget, depthStencil);

        // set the viewport
        auto viewport = mDeviceResources->GetScreenViewport();
        context->RSSetViewports(1, &viewport);

        mDeviceResources->PIXEndEvent();
    }
#pragma endregion

#pragma region Message Handlers
    void EngineWindowsNT6_1::OnActivated()
    {
        // TODO: game is becoming active window
    }

    void EngineWindowsNT6_1::OnDeactivated()
    {
        // TODO: game is becoming background window
    }

    void EngineWindowsNT6_1::OnSuspending()
    {
        // TODO: game is being power-suspended (or minimized)
    }

    void EngineWindowsNT6_1::OnResuming()
    {
        mTimer.ResetElapsedTime();

        // TODO: game is being power-resumed (or returning from minimize)
    }

    void EngineWindowsNT6_1::OnWindowMoved()
    {
        auto r = mDeviceResources->GetOutputSize();
        mDeviceResources->WindowSizeChanged(r.right, r.bottom);
    }

    void EngineWindowsNT6_1::OnDisplayChange()
    {
        mDeviceResources->UpdateColorSpace();
    }

    void EngineWindowsNT6_1::OnWindowSizeChanged(int width, int height)
    {
        if (!mDeviceResources->WindowSizeChanged(width, height))
            return;

        CreateWindowSizeDependentResources();

        // TODO: game window is being resized
    }

    // properties
    void EngineWindowsNT6_1::GetDefaultSize(int &width, int &height) const noexcept
    {
        // TODO: change to desired default window size (note minimum size is 320x200)
        width = 800;
        height = 600;
    }
#pragma endregion

#pragma region Direct3D Resources
    /// these are the resources that depend on the device.
    void EngineWindowsNT6_1::CreateDeviceDependentResources()
    {
        auto device = mDeviceResources->GetD3DDevice();

        // TODO: Initialize device dependent objects here (independent of window size).
        mEffect = std::make_unique<BasicEffect>(device);
        mEffect->SetTextureEnabled(true);
        mEffect->SetPerPixelLighting(true);
        mEffect->SetLightingEnabled(true);
        mEffect->SetLightEnabled(0, true);
        mEffect->SetLightDiffuseColor(0, Colors::White);
        mEffect->SetLightDirection(0, -Vector3::UnitZ);

        auto context = mDeviceResources->GetD3DDeviceContext();

        mShape = GeometricPrimitive::CreateSphere(context);

#if 0
        mShape = GeometricPrimitive::CreateTorus(context);
#endif

        mShape->CreateInputLayout(mEffect.get(), mInputLayout.ReleaseAndGetAddressOf());

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
            CreateDDSTextureFromMemory(device, context, m_ddsTexture.data(), m_ddsTexture.size(), nullptr, mTexture.ReleaseAndGetAddressOf()));

        mEffect->SetTexture(mTexture.Get());

        mWorld = Matrix::Identity;
    }

    /// allocate all memory resources that change on a window SizeChanged event
    void EngineWindowsNT6_1::CreateWindowSizeDependentResources()
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

    void EngineWindowsNT6_1::OnDeviceLost()
    {
        // TODO: add Direct3D resource cleanup here
        mShape.reset();
        mTexture.Reset();
        mEffect.reset();
        mInputLayout.Reset();
    }

    void EngineWindowsNT6_1::OnDeviceRestored()
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
    return std::make_shared<WindowsNT6_1::EngineWindowsNT6_1>(application);
}

/// debug log, implemented at platform level
void Engine::DebugOutput(const std::string &message)
{
    OutputDebugStringA((message+'\n').c_str());
}
