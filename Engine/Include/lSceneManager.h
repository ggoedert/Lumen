//==============================================================================================================================================================================
/// \file
/// \brief     scene manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lComponent.h"
#include "lEntity.h"
#include "lScene.h"

#include <functional>

/// Lumen namespace
namespace Lumen
{
    CLASS_WEAK_PTR_DEF(Engine);

    /// SceneManager namespace
    namespace SceneManager
    {
        /// component maker function type
        using ComponentMaker = std::function<ComponentPtr(const EngineWeakPtr &engine, const EntityWeakPtr &entity)>;

        /// initialize scene manager namespace
        void Initialize();

        /// shutdown scene manager namespace
        void Shutdown();

        /// register component maker
        void RegisterComponentMaker(HashType type, const ComponentMaker &maker);

        /// load scene
        bool Load(ScenePtr scene);

        /// unload current scene
        void Unload();

        /// get the current scene
        [[nodiscard]] ScenePtr CurrentScene();

        /// create component of a specific type
        ComponentWeakPtr CreateComponent(const EngineWeakPtr &engine, const EntityWeakPtr &entity, Hash type);

        /// register entity in the current scene
        [[nodiscard]] EntityWeakPtr RegisterEntity(const EntityPtr &entity);

        /// unregister entity from the current scene
        bool UnregisterEntity(const EntityWeakPtr &entity);

        /// get the count of entities
        [[nodiscard]] size_t EntityCount();

        /// register component
        [[nodiscard]] ComponentWeakPtr RegisterComponent(const ComponentPtr &component);

        /// unregister component
        bool UnregisterComponent(const ComponentWeakPtr &component);

        /// get the count of components of a specific type
        [[nodiscard]] size_t ComponentCount(Hash type);

        /// get all components of type
        [[nodiscard]] Components GetComponents(Hash type);

        /// run application
        void Run();
    }
}
