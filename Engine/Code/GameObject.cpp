//==============================================================================================================================================================================
/// \file
/// \brief     game object
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lGameObject.h"
#include "lSceneManager.h"

using namespace Lumen;

/// constructs a game object with a scene manager
GameObject::GameObject(SceneManager &sceneManager) : Object(), mSceneManager(sceneManager), mTransform(this)
{
    sceneManager.RegisterGameObject(this);
}

/// destroys game object
GameObject::~GameObject()
{
    for (Component *component : mComponents)
    {
        mSceneManager.UnregisterComponent(component->ComponentType(), component);
        delete component;
    }
    mSceneManager.UnregisterGameObject(this);
}

/// get component
Component *GameObject::GetComponent(Type componentType)
{
    for (Component *component : mComponents)
    {
        if (component->ComponentType() == componentType)
        {
            return component;
        }
    }
    return nullptr;
}

/// add a component
void GameObject::AddComponent(Component *component)
{
    component->SetGameObject(this);
    mComponents.push_back(component);
    mSceneManager.RegisterComponent(component->ComponentType(), component);
}

/// run game object
void GameObject::Run()
{
    for (Component *component : mComponents)
        component->Run();
}

Application &GameObject::GetApplication()
{
    return mSceneManager.GetApplication();
}
