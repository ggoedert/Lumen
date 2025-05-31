//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngineWindows.h"
#include "lApplication.h"

#include "DDS.h"

using namespace DX;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Lumen;

using Microsoft::WRL::ComPtr;

EngineWindows::EngineWindows(std::shared_ptr<Application> application) noexcept(false) :
    mDeviceResources(std::make_unique<DeviceResources>()),
    mApplication(application)
{
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DeviceResources::c_EnableHDR for HDR10 display.
    mDeviceResources->RegisterDeviceNotify(this);
}

EngineWindows::~EngineWindows()
{
    if (mDeviceResources)
    {
        mDeviceResources->WaitForGpu();
    }
}

/// initialize the Direct3D resources required to run
bool EngineWindows::Initialize(HWND window, int width, int height)
{
    mApplication->SetEngine(shared_from_this());

    mDeviceResources->SetWindow(window, width, height);

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

    // load application
    return mApplication->Load();
}

#pragma region Frame Update
/// executes the basic game loop
bool EngineWindows::Tick()
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
bool EngineWindows::Update(StepTimer const &timer)
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
void EngineWindows::Render()
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
void EngineWindows::Clear()
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
void EngineWindows::OnActivated()
{
    // TODO: game is becoming active window
}

void EngineWindows::OnDeactivated()
{
    // TODO: game is becoming background window
}

void EngineWindows::OnSuspending()
{
    // TODO: game is being power-suspended (or minimized)
}

void EngineWindows::OnResuming()
{
    mTimer.ResetElapsedTime();

    // TODO: game is being power-resumed (or returning from minimize)
}

void EngineWindows::OnWindowMoved()
{
    auto r = mDeviceResources->GetOutputSize();
    mDeviceResources->WindowSizeChanged(r.right, r.bottom);
}

void EngineWindows::OnDisplayChange()
{
    mDeviceResources->UpdateColorSpace();
}

void EngineWindows::OnWindowSizeChanged(int width, int height)
{
    if (!mDeviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: game window is being resized
}

// properties
void EngineWindows::GetDefaultSize(int &width, int &height) const noexcept
{
    // TODO: change to desired default window size (note minimum size is 320x200)
    width = 1600;
    height = 900;
}
#pragma endregion

#pragma region Direct3D Resources
/// these are the resources that depend on the device.
void EngineWindows::CreateDeviceDependentResources()
{
    auto device = mDeviceResources->GetD3DDevice();

    // check shader model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
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
#define WIDTH  256
#define HEIGHT 256
#define SCREEN_TEX_PITCH ((256 * 8 * 4 + 7) / 8)

    std::vector<byte> m_ddsTexture(DDS_PREFIX + 256 * 256 * 4);
    *((DWORD *)m_ddsTexture.data()) = DDS_MAGIC;
    DDS_HEADER *header = (DDS_HEADER *)(m_ddsTexture.data() + sizeof(DWORD));

    memset(header, 0, sizeof(DDS_HEADER));
    header->size = sizeof(DDS_HEADER);
    header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_PITCH | DDS_HEADER_FLAGS_MIPMAP;
    header->height = 256;
    header->width = 256;
    header->pitchOrLinearSize = SCREEN_TEX_PITCH;
    header->mipMapCount = 1;
    header->ddspf = DDSPF_A8B8G8R8;
    header->caps = DDS_SURFACE_FLAGS_TEXTURE;

    byte *tex = m_ddsTexture.data() + DDS_PREFIX;
    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            if (y < 128)
            {
                if (x < 128)
                {
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 0] = 255;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 1] = 255;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 2] = 0;
                }
                else
                {
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 0] = 0;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 1] = 128;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 2] = 128;
                }
            }
            else
            {
                if (x < 128)
                {
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 0] = 255;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 1] = 192;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 2] = 203;
                }
                else
                {
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 0] = 255;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 1] = 0;
                    tex[y * SCREEN_TEX_PITCH + x * 4 + 2] = 0;
                }
            }
            tex[y * SCREEN_TEX_PITCH + x * 4 + 3] = 255;
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
void EngineWindows::CreateWindowSizeDependentResources()
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

void EngineWindows::OnDeviceLost()
{
    // TODO: add Direct3D resource cleanup here
    mShape.reset();
    mEffect.reset();
    mTexture.Reset();
    mResourceDescriptors.reset();
    mStates.reset();
    mGraphicsMemory.reset();
}

void EngineWindows::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
