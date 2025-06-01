//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_ENGINE_WINDOWS_H
#define L_ENGINE_WINDOWS_H

#include "lEngine.h"
#include "lApplication.h"

// helpers headers
#include "Framework.h"
#include "DeviceResources.h"
#include "StepTimer.h"

/// Lumen namespace
namespace Lumen
{
    /// Engine windows implementation.
    /// creates a D3D12 device and provides a game loop
    class EngineWindows final : public Engine, public DX::IDeviceNotify
    {
        CLASS_UTILS(EngineWindows);

    public:
        EngineWindows(Application::Ptr application) noexcept(false);
        ~EngineWindows();

        // initialization and management
        bool Initialize(HWND window, int width, int height);

        // basic game loop
        bool Tick();

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

        // properties
        void GetDefaultSize(int &width, int &height) const noexcept;

    private:
        bool Update(StepTimer const &timer);
        void Render();

        void Clear();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

        // application
        Application::Ptr mApplication;

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

#endif
