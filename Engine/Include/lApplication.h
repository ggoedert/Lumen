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

        /// load application
        virtual bool Load() = 0;

        /// unload application
        virtual void Unload() = 0;

        /// get delta time
        [[nodiscard]] float GetDeltaTime() const;

        /// get time
        [[nodiscard]] float GetTime() const;

        /// get background color
        [[nodiscard]] Math::Vector GetBackgroundColor() const;

    protected:
        /// constructs application
        Application();

        /// virtual destructor
        virtual ~Application() noexcept;

        /// run application
        bool Run(float deltaTime);

        /// quit application
        void Quit();

    private:
        CLASS_PIMPL_DEF(Impl);
    };
}
