//==============================================================================================================================================================================
/// \file
/// \brief     scene manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lComponent.h"
#include "lGameObject.h"
#include "lScene.h"
#include "lApplication.h"

#include <vector>
#include <map>

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEFS(SceneManager);

    /// SceneManager class
    class SceneManager
    {
        CLASS_NO_DEFAULT_CTOR(SceneManager);
        CLASS_NO_COPY_MOVE(SceneManager);

    public:
        /// constructs a scene manager with a application
        SceneManager(Application &application) : mApplication(application) {}

        /// load scene
        bool Load(ScenePtr scene);

        /// unload current scene
        void Unload();

        /// register game object in the current scene
        void RegisterGameObject(GameObject *gameObject);

        /// unregister game object from the current scene
        void UnregisterGameObject(GameObject *gameObject);

        /// register component
        void RegisterComponent(Type componentType, Component *component);

        /// unregister component
        void UnregisterComponent(Type componentType, Component *component);

        /// get all components of type
        std::vector<Component *> &GetComponents(Type componentType);

        /// run current scene
        void Run();

        /// get application
        [[nodiscard]] Application &GetApplication() { return mApplication; }

    private:
        /// application
        Application &mApplication;

        /// current loaded scene
        ScenePtr mCurrentScene;

        /// game objects in the scene
        std::vector<GameObject *> mGameObjects;

        /// components that need to be started
        std::vector<Component *> mNewComponents;

        /// map of components
        std::map<Type, std::vector<Component *>> mComponentsMap;
    };
}
