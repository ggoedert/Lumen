//==============================================================================================================================================================================
/// \file
/// \brief     scene manager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lSceneManager.h"
#include "lCamera.h"
#include "lMeshFilter.h"

using namespace Lumen;

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

/// initialize scene manager namespace
void SceneManager::Initialize()
{
    LUMEN_ASSERT(!gSceneManagerState);
    gSceneManagerState = std::make_unique<SceneManagerState>();

    SceneManager::RegisterComponentMaker(Camera::Type(), Camera::MakePtr);
    SceneManager::RegisterComponentMaker(MeshFilter::Type(), MeshFilter::MakePtr);
}

/// shutdown scene manager namespace
void SceneManager::Shutdown()
{
    LUMEN_ASSERT(gSceneManagerState);
    Unload();
    gSceneManagerState.reset();
}

/// load scene
bool SceneManager::Load(ScenePtr scene)
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

/// register component maker
void SceneManager::RegisterComponentMaker(const HashType type, const ComponentMaker &maker)
{
    LUMEN_ASSERT(gSceneManagerState);
    LUMEN_ASSERT(!gSceneManagerState->mComponentMakers.contains(type));
    gSceneManagerState->mComponentMakers[type] = maker;
}

/// create component of a specific type
ComponentWeakPtr SceneManager::CreateComponent(const GameObjectWeakPtr &gameObject, const HashType type, const std::any &params)
{
    LUMEN_ASSERT(gSceneManagerState);
    LUMEN_ASSERT(gSceneManagerState->mComponentMakers.contains(type));

    auto it = gSceneManagerState->mComponentMakers.find(type);
    if (it == gSceneManagerState->mComponentMakers.end())
    {
        return ComponentWeakPtr();
    }
    return RegisterComponent(it->second(gameObject, params));
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
    LUMEN_ASSERT(component);

    gSceneManagerState->mComponentsMap[component->Type()].push_back(component);
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

    auto it = gSceneManagerState->mComponentsMap.find(lockedComponent->Type());
    LUMEN_ASSERT(it != gSceneManagerState->mComponentsMap.end());

    return RemoveFromVector(it->second, lockedComponent);
}

/// get all components of type
Components SceneManager::GetComponents(const HashType type)
{
    LUMEN_ASSERT(gSceneManagerState);
    Components result;

    auto it = gSceneManagerState->mComponentsMap.find(type);
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
