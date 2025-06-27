//==============================================================================================================================================================================
/// \file
/// \brief     scene manager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lSceneManager.h"
#include "lCamera.h"
#include "lMeshFilter.h"

using namespace Lumen;

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    struct SceneManagerState
    {
        CLASS_NO_COPY_MOVE(SceneManagerState);

        /// default constructor
        explicit SceneManagerState() = default;

        /// current loaded scene
        ScenePtr mCurrentScene;

        /// map of component makers
        std::map<HashType, SceneManager::ComponentMaker> mComponentMakers;

        /// game objects in the scene
        std::vector<GameObjectPtr> mGameObjects;

        /// components that need to be started
        std::vector<ComponentPtr> mNewComponents;

        /// map of components
        std::map<HashType, std::vector<ComponentPtr>> mComponentsMap;
    };

    static std::unique_ptr<SceneManagerState> gSceneManagerState;
}

/// initialize scene manager namespace
void SceneManager::Initialize()
{
    LUMEN_ASSERT(!Hidden::gSceneManagerState);
    Hidden::gSceneManagerState = std::make_unique<Hidden::SceneManagerState>();

    SceneManager::RegisterComponentMaker(Camera::Type(), Camera::MakePtr);
    SceneManager::RegisterComponentMaker(MeshFilter::Type(), MeshFilter::MakePtr);
}

/// shutdown scene manager namespace
void SceneManager::Shutdown()
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);
    Unload();
    Hidden::gSceneManagerState.reset();
}

/// load scene
bool SceneManager::Load(ScenePtr scene)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);
    Unload();
    Hidden::gSceneManagerState->mCurrentScene = std::move(scene);
    return Hidden::gSceneManagerState->mCurrentScene->Load();
}

/// unload current scene
void SceneManager::Unload()
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);
    if (Hidden::gSceneManagerState->mCurrentScene)
    {
        Hidden::gSceneManagerState->mCurrentScene->Unload();
        Hidden::gSceneManagerState->mNewComponents.clear();
        Hidden::gSceneManagerState->mGameObjects.clear();
        Hidden::gSceneManagerState->mComponentsMap.clear();
        Hidden::gSceneManagerState->mCurrentScene.reset();

    }
}

/// register component maker
void SceneManager::RegisterComponentMaker(const HashType type, const ComponentMaker &maker)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);
    LUMEN_ASSERT(!Hidden::gSceneManagerState->mComponentMakers.contains(type));
    Hidden::gSceneManagerState->mComponentMakers[type] = maker;
}

/// create component of a specific type
ComponentWeakPtr SceneManager::CreateComponent(const GameObjectWeakPtr &gameObject, const HashType type, const std::any &params)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);
    LUMEN_ASSERT(Hidden::gSceneManagerState->mComponentMakers.contains(type));

    auto it = Hidden::gSceneManagerState->mComponentMakers.find(type);
    if (it == Hidden::gSceneManagerState->mComponentMakers.end())
    {
        return ComponentWeakPtr();
    }
    return RegisterComponent(it->second(gameObject, params));
}

/// register game object in the current scene
GameObjectWeakPtr SceneManager::RegisterGameObject(const GameObjectPtr &gameObject)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);

    Hidden::gSceneManagerState->mGameObjects.push_back(gameObject);
    return gameObject;
}

/// unregister game object from the current scene
/// the passed GameObjectWeakPtr must have been originally created from a shared GameObjectPtr stored in the SceneManager
bool SceneManager::UnregisterGameObject(const GameObjectWeakPtr &gameObject)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);

    auto lockedGameObject = gameObject.lock();
    if (!lockedGameObject)
    {
        return false;
    }
    return RemoveFromVector(Hidden::gSceneManagerState->mGameObjects, lockedGameObject);
}

/// register component
ComponentWeakPtr SceneManager::RegisterComponent(const ComponentPtr &component)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);
    LUMEN_ASSERT(component);

    Hidden::gSceneManagerState->mComponentsMap[component->Type()].push_back(component);
    Hidden::gSceneManagerState->mNewComponents.push_back(component);
    return component;
}

/// unregister component
bool SceneManager::UnregisterComponent(const ComponentWeakPtr &component)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);

    auto lockedComponent = component.lock();
    if (!lockedComponent)
    {
        return false;
    }

    auto it = Hidden::gSceneManagerState->mComponentsMap.find(lockedComponent->Type());
    LUMEN_ASSERT(it != Hidden::gSceneManagerState->mComponentsMap.end());

    return RemoveFromVector(it->second, lockedComponent);
}

/// get all components of type
Components SceneManager::GetComponents(const HashType type)
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);
    Components result;

    auto it = Hidden::gSceneManagerState->mComponentsMap.find(type);
    if (it == Hidden::gSceneManagerState->mComponentsMap.end())
    {
        return result;
    }

    const auto &components = it->second;
    result.reserve(components.size());
    std::transform(components.begin(), components.end(), std::back_inserter(result),
        [](const ComponentPtr &comp) { return comp; });

    return result;
}

/// run application
void SceneManager::Run()
{
    LUMEN_ASSERT(Hidden::gSceneManagerState);

    if (!Hidden::gSceneManagerState->mNewComponents.empty())
    {
        for (const ComponentPtr &component : Hidden::gSceneManagerState->mNewComponents)
        {
            component->Start();
        }
        Hidden::gSceneManagerState->mNewComponents.clear();
    }

    for (const GameObjectPtr &gameObject : Hidden::gSceneManagerState->mGameObjects)
    {
        gameObject->Run();
    }
}
