//==============================================================================================================================================================================
/// \file
/// \brief     scene manager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lSceneManager.h"

using namespace Lumen;

/// load scene
bool SceneManager::Load(ScenePtr scene)
{
    //unload last scene
    Unload();

    //load current scene
    mCurrentScene = std::move(scene);
    return mCurrentScene->Load();
}

/// unload current scene
void SceneManager::Unload()
{
    if (mCurrentScene)
    {
        mCurrentScene->Unload();
        mCurrentScene.reset();
    }
}

/// register game object in the current scene
void SceneManager::RegisterGameObject(GameObject *gameObject)
{
    mGameObjects.push_back(gameObject);
}

/// unregister game object from the current scene
void SceneManager::UnregisterGameObject(GameObject *gameObject)
{
    mGameObjects.erase(std::remove(mGameObjects.begin(), mGameObjects.end(), gameObject), mGameObjects.end());
}

/// register component
void SceneManager::RegisterComponent(Type componentType, Component *component)
{
    std::map<Type, std::vector<Component *>>::iterator componentsMapIterator = mComponentsMap.find(componentType);
    if (componentsMapIterator == mComponentsMap.end())
    {
        componentsMapIterator = mComponentsMap.insert({ componentType, std::vector<Component *>() }).first;
    }
    std::vector<Component *> &componentVector = componentsMapIterator->second;
    componentVector.push_back(component);
    mNewComponents.push_back(component);
}

/// unregister component
void SceneManager::UnregisterComponent(Type componentType, Component *component)
{
    std::map<Type, std::vector<Component *>>::iterator componentsMapIterator = mComponentsMap.find(componentType);
    assert(componentsMapIterator != mComponentsMap.end());
    std::vector<Component *> &componentVector = componentsMapIterator->second;
    componentVector.erase(std::remove(componentVector.begin(), componentVector.end(), component), componentVector.end());
}

/// get all components of type
std::vector<Component *> &SceneManager::GetComponents(Type componentType)
{
    std::map<Type, std::vector<Component *>>::iterator componentsMapIterator = mComponentsMap.find(componentType);
    if (componentsMapIterator == mComponentsMap.end())
    {
        componentsMapIterator = mComponentsMap.insert({ componentType, std::vector<Component *>() }).first;
    }
    return componentsMapIterator->second;
}

/// run current scene
void SceneManager::Run()
{
    for (Component *component : mNewComponents)
    {
        component->Start();
    }
    mNewComponents.clear();

    for (GameObject *gameObject : mGameObjects)
    {
        gameObject->Run();
    }
}
