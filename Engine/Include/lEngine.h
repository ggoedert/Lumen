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
    class Application;
    CLASS_PTR_DEFS(Application);
    CLASS_PTR_DEFS(Engine);

    /// Engine class
    class Engine : public std::enable_shared_from_this<Engine>
    {
    public:
        /// virtual destructor
        virtual ~Engine() = default;

        // allocate smart pointer version of the engine, implemented at platform level
        static EnginePtr MakePtr(ApplicationPtr application);

        // initialization and management
        virtual bool Initialize(std::any config) = 0;

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
        /// protected default constructor, allows construction by smart pointer
        Engine() = default;
    };
}
