//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lApplication.h"
#include "lEnginePlatform.h"
#include "lEngine.h"

// helpers headers
#include "DeviceResources.h"
#include "StepTimer.h"

/// Lumen namespace
namespace Lumen
{
    /// Engine windows implementation.
    /// creates a D3D12 device and provides a game loop
    class EngineWindows final : public Engine, public DX::IDeviceNotify
    {
        CLASS_NO_DEFAULT_CTOR(EngineWindows);
        CLASS_NO_COPY_MOVE(EngineWindows);

    public:
        EngineWindows(const ApplicationPtr &application) noexcept(false);
        ~EngineWindows();

        // initialization and management
        bool Initialize(const std::any &config) override;

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
};
