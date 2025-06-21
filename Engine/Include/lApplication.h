//==============================================================================================================================================================================
/// \file
/// \brief     Application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lMath.h"
#include "lSceneManager.h"
#include "lEngine.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Engine);

    /// Application class
    class Application
    {
        friend class Engine;
        CLASS_NO_COPY_MOVE(Application);

    public:
        /// set engine
        void SetEngine(const EnginePtr &engine);

        /// initialize application
        virtual bool Initialize() = 0;

        /// shutdown application
        virtual void Shutdown() = 0;

        /// get delta time
        [[nodiscard]] float DeltaTime() const;

        /// get time
        [[nodiscard]] float Time() const;

        /// get application's background color
        [[nodiscard]] const Math::Vector BackgroundColor() const;

    protected:
        /// constructs application
        explicit Application();

        /// virtual destructor
        virtual ~Application() noexcept;

        /// run application
        bool Run(float deltaTime);

        /// quit application
        void Quit();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
