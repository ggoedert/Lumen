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
        ~EngineWindowsNT10() override;

        // initialization and management
        bool Initialize(const Object &config) override;

        /// shutdown
        void Shutdown() override;

        // get elapsed time since last run
        float GetElapsedTime() override;

        // basic game loop
        bool Run(std::function<bool()> update) override;

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
        IFileSystemPtr FolderFileSystem(std::string_view name, const std::filesystem::path &path) const override
        {
            return FolderFileSystem::MakePtr(name, path);
        }

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

    private:
        void Render();

        void Clear();

        void CreateNewDeviceDependentResources();
        void CreateNewWindowSizeDependentResources();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

        // device resources
        std::unique_ptr<DX::DeviceResources> mDeviceResources;

        // rendering loop timer
        StepTimer mTimer;

        std::unique_ptr<GraphicsMemory> mGraphicsMemory;
        std::unique_ptr<CommonStates> mStates;

        SimpleMath::Matrix mView;
        SimpleMath::Matrix mProj;

        std::unique_ptr<DynamicDescriptorHeap> mResourceDescriptors;

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
            DynamicDescriptorHeap::IndexType mIndex;
            Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
            int mWidth;
            int mHeight;
        };
        using TextureMapType = std::unordered_map<Id::Type, TextureData>;
        TextureMapType mTextureMap;
        std::vector<TextureMapType::iterator> mNewDeviceTextureMap;

        std::vector<Engine::RenderCommand> mRenderCommands;
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
    bool EngineWindowsNT10::Run(std::function<bool()> update)
    {
        bool updateResult = true;

        mTimer.Tick([&]()
        {
            if (updateResult)
            {
                PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");
                updateResult = update();
                PIXEndEvent();
            }
        });
        if (!updateResult)
            return false;

        Render();
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

        // prepare the command list to render a new frame
        mDeviceResources->Prepare();
        Clear();

        auto commandList = mDeviceResources->GetCommandList();
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

        // TODO: add your rendering code here
        ID3D12DescriptorHeap *heaps[] = { mResourceDescriptors->Heap(), mStates->Heap() };
        commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

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
        textureData.mHeight = width;
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

            // create texture
            textureDataIt->second.mTexture->GetTextureData(ddsTexture.data() + ddsPrefix, screenTexPitch);
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
        for (auto &shaderDataIt : mNewWindowShaderMap)
        {
            BasicEffect *basicEffect = static_cast<BasicEffect *>(shaderDataIt->second.mEffect.get());
            basicEffect->SetView(mView);
            basicEffect->SetProjection(mProj);
        }

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

        CreateNewDeviceDependentResources();
    }

    /// allocate all memory resources that change on a window SizeChanged event
    void EngineWindowsNT10::CreateWindowSizeDependentResources()
    {
        // TODO: initialize windows-size dependent objects here
        auto size = mDeviceResources->GetOutputSize();

        mView = Matrix::CreateLookAt(Vector3(0.f, 1.f, -2.f),
            Vector3::Zero, Vector3::UnitY);
        mProj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
            float(size.right) / float(size.bottom), 0.1f, 10.f);

        CreateNewWindowSizeDependentResources();
    }

    void EngineWindowsNT10::OnDeviceLost()
    {
        // TODO: add Direct3D resource cleanup here
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
}

//==============================================================================================================================================================================

/// allocate smart pointer version of the engine, implemented at platform level
EnginePtr Engine::MakePtr(const ApplicationPtr &application)
{
    return EnginePtr(new Engine(application, new WindowsNT10::EngineWindowsNT10()));
}

/// debug log, implemented at platform level
void Engine::DebugOutput(const std::string &message)
{
    OutputDebugStringA((message+'\n').c_str());
}
