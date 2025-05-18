//
// Engine.h
//

#pragma once

/// Lumen namespace
namespace Lumen
{

    class Engine
    {
    public:

        Engine() noexcept(false);
        ~Engine();

        Engine(Engine &&) = default;
        Engine &operator= (Engine &&) = default;

        Engine(Engine const &) = delete;
        Engine &operator= (Engine const &) = delete;

        // Initialization and management
        void Initialize(HWND window, int width, int height);

        // Basic engine loop
        void Tick();

        // Messages
        void OnActivated();
        void OnDeactivated();
        void OnSuspending();
        void OnResuming();
        void OnWindowMoved();
        void OnWindowSizeChanged(int width, int height);

        // Properties
        void GetDefaultSize(int &width, int &height) const noexcept;
    };

    Engine *GetEngine();
}
