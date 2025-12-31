//==============================================================================================================================================================================
/// \file
/// \brief     Application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lMath.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Application);
#ifdef EDITOR
    CLASS_PTR_DEF(Editor);
#endif

    /// Application class
    class Application
    {
        friend class Engine;
        CLASS_NO_COPY_MOVE(Application);

    public:
        /// application state
        enum class State { Running, Stepping, Stopping, Stopped, Quit };

        /// get engine
        [[nodiscard]] EngineWeakPtr GetEngine();

        /// set engine
        void SetEngine(const EngineWeakPtr &engine);

        /// initialize application
        virtual void Initialize(const ApplicationWeakPtr &application);

        /// shutdown application
        virtual void Shutdown();

        /// get winwdow size
        virtual void GetWindowSize(int &width, int &height) = 0;

        /// TEMP DELME!
        virtual void New() = 0;

        /// TEMP DELME!
        virtual void Open() = 0;

        /// get delta time
        [[nodiscard]] float DeltaTime() const;

        /// get time
        [[nodiscard]] float Time() const;

        /// get application's background color
        [[nodiscard]] const Math::Vector4 &BackgroundColor() const;

        /// quit application
        void Quit();

#ifdef EDITOR
        /// get application state
        [[nodiscard]] State GetState();

        /// get paused state
        [[nodiscard]] bool Paused();

        /// start application
        void Start();

        /// step application
        void Step();

        /// pause application
        void Pause();

        /// stop application
        void Stop();
#endif

    protected:
        /// constructs application
        explicit Application();

        /// virtual destructor
        virtual ~Application();

#ifdef EDITOR
        /// run editor
        void Editor();
#endif

        /// run application
        bool Run(float deltaTime);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
