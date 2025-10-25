//==============================================================================================================================================================================
/// \file
/// \brief     Application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lMath.h"
#include "lAssets.h"

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
        /// get engine
        [[nodiscard]] EngineWeakPtr GetEngine();

        /// set engine
        void SetEngine(const EngineWeakPtr &engine);

        /// initialize application
        virtual void Initialize();

        /// shutdown aplication
        virtual void Shutdown();

        /// TEMP DELME!
        virtual void New() = 0;

        /// TEMP DELME!
        virtual void Open() = 0;

        /// get delta time
        [[nodiscard]] float DeltaTime() const;

        /// get time
        [[nodiscard]] float Time() const;

        /// get application's background color
        [[nodiscard]] const Math::Vector &BackgroundColor() const;

    protected:
        /// constructs application
        explicit Application();

        /// virtual destructor
        virtual ~Application();

        /// run application
        bool Run(float deltaTime);

        /// quit application
        void Quit();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
