//==============================================================================================================================================================================
/// \file
/// \brief     scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_SCENE_H
#define L_SCENE_H

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    class Application;

    /// Scene class
    class Scene
    {
        CLASS_UTILS(Scene);

    public:
        /// constructor
        Scene(Application &application) : mApplication(application) {}

        /// load scene
        virtual bool Load() = 0;

        /// unload scene
        virtual void Unload() = 0;

    protected:

        /// application
        Application &mApplication;
    };
}

#endif
