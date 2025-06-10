//==============================================================================================================================================================================
/// \file
/// \brief     scene manager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lSceneManager.h"

using namespace Lumen;

struct SceneManagerState
{
    CLASS_NO_COPY_MOVE(SceneManagerState);

    /// default constructor
    SceneManagerState() = default;

    /// current loaded scene
    ScenePtr mCurrentScene;

    /// game objects in the scene
    std::vector<GameObjectPtr> mGameObjects;

    /// components that need to be started
    std::vector<ComponentPtr> mNewComponents;

    /// map of components
    std::map<Type, std::vector<ComponentPtr>> mComponentsMap;
};

SceneManagerState *gSceneManagerState = nullptr;

/// initialize scene manager namespace
void SceneManager::Initialize()
{
    LUMEN_ASSERT(gSceneManagerState == nullptr);
    gSceneManagerState = new SceneManagerState();
}

/// shutdown scene manager namespace
void SceneManager::Shutdown()
{
    LUMEN_ASSERT(gSceneManagerState);
    Unload();
    delete gSceneManagerState;
    gSceneManagerState = nullptr;
}

/// load scene
bool SceneManager::Load(const ScenePtr &scene)
{
    LUMEN_ASSERT(gSceneManagerState);
    Unload();
    gSceneManagerState->mCurrentScene = std::move(scene);
    return gSceneManagerState->mCurrentScene->Load();
}

/// unload current scene
void SceneManager::Unload()
{
    LUMEN_ASSERT(gSceneManagerState);
    if (gSceneManagerState->mCurrentScene)
    {
        gSceneManagerState->mCurrentScene->Unload();
        gSceneManagerState->mNewComponents.clear();
        gSceneManagerState->mGameObjects.clear();
        gSceneManagerState->mComponentsMap.clear();
        gSceneManagerState->mCurrentScene.reset();

    }
}

/// register game object in the current scene
GameObjectWeakPtr SceneManager::RegisterGameObject(const GameObjectPtr &gameObject)
{
    LUMEN_ASSERT(gSceneManagerState);

    gSceneManagerState->mGameObjects.push_back(gameObject);
    return gameObject;
}

/// unregister game object from the current scene
/// the passed GameObjectWeakPtr must have been originally created from a shared GameObjectPtr stored in the SceneManager
bool SceneManager::UnregisterGameObject(const GameObjectWeakPtr &gameObject)
{
    LUMEN_ASSERT(gSceneManagerState);

    auto lockedGameObject = gameObject.lock();
    if (!lockedGameObject)
    {
        return false;
    }
    return RemoveFromVector(gSceneManagerState->mGameObjects, lockedGameObject);
}

/// register component
ComponentWeakPtr SceneManager::RegisterComponent(const ComponentPtr &component)
{
    LUMEN_ASSERT(gSceneManagerState);

    gSceneManagerState->mComponentsMap[component->ComponentType()].push_back(component);
    gSceneManagerState->mNewComponents.push_back(component);
    return component;
}

/// unregister component
bool SceneManager::UnregisterComponent(const ComponentWeakPtr &component)
{
    LUMEN_ASSERT(gSceneManagerState);

    auto lockedComponent = component.lock();
    if (!lockedComponent)
    {
        return false;
    }

    auto it = gSceneManagerState->mComponentsMap.find(lockedComponent->ComponentType());
    LUMEN_ASSERT(it != gSceneManagerState->mComponentsMap.end());

    return RemoveFromVector(it->second, lockedComponent);
}

/// get all components of type
Components SceneManager::GetComponents(Type componentType)
{
    LUMEN_ASSERT(gSceneManagerState);
    Components result;

    auto it = gSceneManagerState->mComponentsMap.find(componentType);
    if (it == gSceneManagerState->mComponentsMap.end())
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
    LUMEN_ASSERT(gSceneManagerState);

    if (!gSceneManagerState->mNewComponents.empty())
    {
        for (const ComponentPtr &component : gSceneManagerState->mNewComponents)
        {
            component->Start();
        }
        gSceneManagerState->mNewComponents.clear();
    }

    for (const GameObjectPtr &gameObject : gSceneManagerState->mGameObjects)
    {
        gameObject->Run();
    }
}
