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

#include <functional>

/// Lumen namespace
namespace Lumen
{
    /// SceneManager namespace
    namespace SceneManager
    {
        /// component maker function type
        using ComponentMaker = std::function<ComponentPtr(const std::any &params)>;

        /// initialize scene manager namespace
        void Initialize();

        /// shutdown scene manager namespace
        void Shutdown();

        /// load scene
        bool Load(ScenePtr scene);

        /// unload current scene
        void Unload();

        /// get the current scene
        [[nodiscard]] ScenePtr CurrentScene();

        /// register component maker
        void RegisterComponentMaker(const Type type, const ComponentMaker &maker);

        /// create component of a specific type
        ComponentWeakPtr CreateComponent(const Type type, const std::any &params);

        /// register game object in the current scene
        [[nodiscard]] GameObjectWeakPtr RegisterGameObject(const GameObjectPtr &gameObject);

        /// unregister game object from the current scene
        bool UnregisterGameObject(const GameObjectWeakPtr &gameObject);

        /// get the count of game objects
        [[nodiscard]] size_t GameObjectCount();

        /// register component
        [[nodiscard]] ComponentWeakPtr RegisterComponent(const ComponentPtr &component);

        /// unregister component
        bool UnregisterComponent(const ComponentWeakPtr &component);

        /// get the count of components of a specific type
        [[nodiscard]] size_t ComponentCount(const Type type);

        /// get all components of type
        [[nodiscard]] Components GetComponents(const Type type);

        /// run application
        void Run();
    };
}
