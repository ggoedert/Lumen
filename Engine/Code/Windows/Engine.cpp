//==============================================================================================================================================================================
/// \file
/// \brief     Engine implementation class
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

// helper
#include "lFramework.h"

#include "DDS.h"

// engine interfaces
#include "lEngine.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Lumen::Engine::Engine(std::unique_ptr<Application> &&application) noexcept(false)
{
    m_application = std::move(application);  // move from temporary or argument
    m_deviceResources = std::make_unique<DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Lumen::Engine::~Engine()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

/// initialize the Direct3D resources required to run
void Lumen::Engine::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: change the timer settings if you want something other than the default variable timestep mode
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
/// executes the basic game loop
bool Lumen::Engine::Tick()
{
    bool updateResult = true;

    m_timer.Tick([&]()
    {
        if (updateResult)
            updateResult = Update(m_timer);
    });
    if (!updateResult)
        return false;

    Render();
    return true;
}

/// updates the world
bool Lumen::Engine::Update(StepTimer const &timer)
{
    bool runResult = true;

    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    auto time = static_cast<float>(timer.GetTotalSeconds());

    // TODO: add your game logic here
#if 0
    m_world = Matrix::CreateRotationZ(cosf(time) * 2.f);
#endif

#if 1
    m_world = Matrix::CreateRotationY(time);
#endif

    PIXEndEvent();

    return runResult;
}
#pragma endregion

#pragma region Frame Render
/// draws the scene
void Lumen::Engine::Render()
{
    // don't try to render anything before the first Update
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // prepare the command list to render a new frame
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // TODO: add your rendering code here
    ID3D12DescriptorHeap *heaps[] = { m_resourceDescriptors->Heap(), m_states->Heap() };
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

    m_effect->SetWorld(m_world);

    m_effect->Apply(commandList);

    m_shape->Draw(commandList);

    PIXEndEvent(commandList);

    // show the new frame
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
    PIXEndEvent();
}

/// helper method to clear the back buffers
void Lumen::Engine::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // clear the views
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // set the viewport and scissor rect
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
void Lumen::Engine::OnActivated()
{
    // TODO: game is becoming active window
}

void Lumen::Engine::OnDeactivated()
{
    // TODO: game is becoming background window
}

void Lumen::Engine::OnSuspending()
{
    // TODO: game is being power-suspended (or minimized)
}

void Lumen::Engine::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: game is being power-resumed (or returning from minimize)
}

void Lumen::Engine::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Lumen::Engine::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Lumen::Engine::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: game window is being resized
}

// properties
void Lumen::Engine::GetDefaultSize(int &width, int &height) const noexcept
{
    // TODO: change to desired default window size (note minimum size is 320x200)
    width = 1920;
    height = 1080;
}
#pragma endregion

#pragma region Direct3D Resources
/// these are the resources that depend on the device.
void Lumen::Engine::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

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
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    m_states = std::make_unique<CommonStates>(device);

    m_resourceDescriptors = std::make_unique<DescriptorHeap>(device,
        Descriptors::Count);

    m_shape = GeometricPrimitive::CreateSphere();

#if 0
    m_shape = GeometricPrimitive::CreateTorus();
#endif

    ResourceUploadBatch resourceUpload(device);

    resourceUpload.Begin();

#define DDS_PREFIX (sizeof(DWORD) + sizeof(DirectX::DDS_HEADER))
#define WIDTH  256
#define HEIGHT 256
#define SCREEN_TEX_PITCH ((256 * 8 * 4 + 7) / 8)

    byte m_ddsTexture[DDS_PREFIX + 256 * 256 * 4];
    *((DWORD *)m_ddsTexture) = DDS_MAGIC;
    DDS_HEADER *header = (DDS_HEADER *)(m_ddsTexture + sizeof(DWORD));

    memset(header, 0, sizeof(DDS_HEADER));
    header->size = sizeof(DDS_HEADER);
    header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_PITCH | DDS_HEADER_FLAGS_MIPMAP;
    header->height = 256;
    header->width = 256;
    header->pitchOrLinearSize = SCREEN_TEX_PITCH;
    header->mipMapCount = 1;
    header->ddspf = DDSPF_A8B8G8R8;
    header->caps = DDS_SURFACE_FLAGS_TEXTURE;

    byte *tex = m_ddsTexture + DDS_PREFIX;
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
        CreateDDSTextureFromMemory(device, resourceUpload, m_ddsTexture, sizeof(m_ddsTexture), m_texture.ReleaseAndGetAddressOf(), true));

    CreateShaderResourceView(device, m_texture.Get(),
        m_resourceDescriptors->GetCpuHandle(Descriptors::Procedural));

#if 1
    m_shape->LoadStaticBuffers(device, resourceUpload);
#endif

    auto uploadResourcesFinished = resourceUpload.End(
        m_deviceResources->GetCommandQueue());

    uploadResourcesFinished.wait();

    RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
        m_deviceResources->GetDepthBufferFormat());

    EffectPipelineStateDescription pd(
        &GeometricPrimitive::VertexType::InputLayout,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        CommonStates::CullNone,
        rtState);

    //m_effect = std::make_unique<BasicEffect>(device, EffectFlags::Lighting, pd);
    //m_effect = std::make_unique<BasicEffect>(device, EffectFlags::Lighting | EffectFlags::Texture, pd);
    m_effect = std::make_unique<BasicEffect>(device, EffectFlags::PerPixelLighting | EffectFlags::Texture, pd);
    //m_effect->EnableDefaultLighting();
    m_effect->SetLightEnabled(0, true);
    m_effect->SetLightDiffuseColor(0, Colors::White);
    m_effect->SetLightDirection(0, -Vector3::UnitZ);

    m_effect->SetTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::Procedural),
        m_states->AnisotropicWrap());

    m_world = Matrix::Identity;
}

/// allocate all memory resources that change on a window SizeChanged event
void Lumen::Engine::CreateWindowSizeDependentResources()
{
    // TODO: initialize windows-size dependent objects here
    auto size = m_deviceResources->GetOutputSize();

    m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
        Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
        float(size.right) / float(size.bottom), 0.1f, 10.f);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
}

void Lumen::Engine::OnDeviceLost()
{
    // TODO: add Direct3D resource cleanup here
    m_shape.reset();
    m_effect.reset();
    m_texture.Reset();
    m_resourceDescriptors.reset();
    m_states.reset();
    m_graphicsMemory.reset();
}

void Lumen::Engine::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
