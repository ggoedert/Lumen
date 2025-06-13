//==============================================================================================================================================================================
/// \file
/// \brief     Engine interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lApplication.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Application);

    /// Engine class
    class Engine : public std::enable_shared_from_this<Engine>
    {
        CLASS_NO_DEFAULT_CTOR(Engine);
        CLASS_NO_COPY_MOVE(Engine);

    public:
        /// virtual destructor
        virtual ~Engine() = default;

        // allocate smart pointer version of the engine, implemented at platform level
        static EnginePtr MakePtr(const ApplicationPtr &application);

        // initialization and management
        virtual bool Initialize(const std::any &config);

        /// shutdown
        virtual void Shutdown();

        // basic game loop
        virtual bool Tick() = 0;

        // messages
        virtual void OnActivated() = 0;
        virtual void OnDeactivated() = 0;
        virtual void OnSuspending() = 0;
        virtual void OnResuming() = 0;
        virtual void OnWindowMoved() = 0;
        virtual void OnDisplayChange() = 0;
        virtual void OnWindowSizeChanged(int width, int height) = 0;

        // properties
        virtual void GetDefaultSize(int &width, int &height) const noexcept = 0;

    protected:
        /// protected constructor
        Engine(const ApplicationPtr &application) : mApplication(application) {}

        /// run engine
        bool Run(float deltaTime);

        // application
        ApplicationPtr mApplication;
    };
}
