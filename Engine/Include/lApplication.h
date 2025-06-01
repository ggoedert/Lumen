//==============================================================================================================================================================================
/// \file
/// \brief     Application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_APPLICATION_H
#define L_APPLICATION_H

#include "lEngine.h"
#include "lSceneManager.h"

/// Lumen namespace
namespace Lumen
{
    /// Application class
    class Application
    {
        CLASS_UTILS(Application);

    public:
        /// constructor
        Application();

        /// virtual destructor
        virtual ~Application() {}

        /// set engine
        void SetEngine(Engine::Ptr engine) { mEngine = engine; }

        /// load application
        virtual bool Load() = 0;

        /// unload application
        virtual void Unload() = 0;

    protected:
        /// Scene manager
        SceneManager mSceneManager;

    private:
        /// engine pointer
        Engine::WeakPtr mEngine;
    };
}

#endif
