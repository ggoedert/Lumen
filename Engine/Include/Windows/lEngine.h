//==============================================================================================================================================================================
/// \file
/// \brief     Engine implementation class
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_ENGINE_H
#define L_ENGINE_H

// helpers headers
#include "lApplication.h"
#include "lDeviceResources.h"
#include "lStepTimer.h"

/// Lumen namespace
namespace Lumen
{
    /// Engine implementation class
    /// creates a D3D12 device and provides a game loop
    class Engine final : public IDeviceNotify
    {
    public:

        Engine() = delete;
        Engine(std::unique_ptr<Application> &&application) noexcept(false);
        ~Engine();

        Engine(Engine &&) = default;
        Engine &operator= (Engine &&) = default;

        Engine(Engine const &) = delete;
        Engine &operator= (Engine const &) = delete;

        // initialization and management
        void Initialize(HWND window, int width, int height);

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
        std::unique_ptr<Application> m_application;

        // device resources
        std::unique_ptr<DeviceResources>    m_deviceResources;

        // rendering loop timer
        StepTimer                           m_timer;

        std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
        std::unique_ptr<DirectX::CommonStates> m_states;

        DirectX::SimpleMath::Matrix m_world;
        DirectX::SimpleMath::Matrix m_view;
        DirectX::SimpleMath::Matrix m_proj;

        std::unique_ptr<DirectX::GeometricPrimitive> m_shape;

        std::unique_ptr<DirectX::BasicEffect> m_effect;

        std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;

        enum Descriptors : size_t
        {
            Procedural,
            Count
        };
    };
};

#endif
