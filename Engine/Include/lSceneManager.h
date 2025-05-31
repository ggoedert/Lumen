//==============================================================================================================================================================================
/// \file
/// \brief     scene manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_SCENE_MANAGER_H
#define L_SCENE_MANAGER_H

#include <memory>

/// Lumen namespace
namespace Lumen
{
    class Application;
    class Scene;

    /// SceneManager class
    class SceneManager
    {
    public:
        /// constructor
        SceneManager(Application &application) : mApplication(application), mCurrentScene(nullptr) {}

        /// load scene
        bool Load(std::shared_ptr<Scene> scene);

        /// unload current scene
        void Unload();

    private:
        /// application
        Application &mApplication;

        /// current loaded scene
        std::shared_ptr<Scene> mCurrentScene;
    };
}

#endif
