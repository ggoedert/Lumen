//==============================================================================================================================================================================
/// \file
/// \brief     scene manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_SCENE_MANAGER_H
#define L_SCENE_MANAGER_H

#include "lScene.h"

/// Lumen namespace
namespace Lumen
{
    class Application;

    /// SceneManager class
    class SceneManager
    {
    public:
        /// constructor
        SceneManager(Application &application) : mApplication(application) {}

        /// load scene
        bool Load(Scene::Ptr scene);

        /// unload current scene
        void Unload();

    private:
        /// application
        Application &mApplication;

        /// current loaded scene
        Scene::Ptr mCurrentScene;
    };
}

#endif
