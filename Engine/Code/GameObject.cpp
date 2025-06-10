//==============================================================================================================================================================================
/// \file
/// \brief     game object
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lGameObject.h"
#include "lTransform.h"
#include "lSceneManager.h"

using namespace Lumen;

/// GameObject::Impl class
class GameObject::Impl
{
    CLASS_NO_DEFAULT_CTOR(GameObject::Impl);
    CLASS_NO_COPY_MOVE(GameObject::Impl);
    CLASS_PTR_UNIQUEMAKER(GameObject::Impl);
    friend class GameObject;

public:
    /// constructs a game object
    Impl(GameObjectWeakPtr &gameObject);

    /// destroys game object
    ~Impl();

    /// get transform
    [[nodiscard]] const Transform &GetTransform() const { return mTransform; }

    /// get component
    [[nodiscard]] ComponentWeakPtr GetComponent(Type componentType);

    /// add a component
    void AddComponent(const ComponentWeakPtr &component);

protected:
    /// run game object
    void Run();

private:
    //game object parent
    GameObjectWeakPtr mGameObject;

    /// transform
    Transform mTransform;

    /// components
    std::vector<ComponentWeakPtr> mComponents;
};

/// constructs a game object
GameObject::Impl::Impl(GameObjectWeakPtr &gameObject) : mGameObject(gameObject), mTransform(gameObject)
{
}

/// destroys game object
GameObject::Impl::~Impl()
{
    for (const ComponentWeakPtr component : mComponents)
    {
        SceneManager::UnregisterComponent(component);
    }
    SceneManager::UnregisterGameObject(mGameObject);
}

/// get component
ComponentWeakPtr GameObject::Impl::GetComponent(Type componentType)
{
    for (const ComponentWeakPtr component : mComponents)
    {
        ComponentPtr c = component.lock();
        LUMEN_ASSERT(c);
        if (c->ComponentType() == componentType)
        {
            return c;
        }
    }
    return {};
}

/// add a component
void GameObject::Impl::AddComponent(const ComponentWeakPtr &component)
{
    auto componentPtr = component.lock();
    if (componentPtr)
    {
        SceneManager::RegisterComponent(componentPtr);
        mComponents.push_back(component);
    }
}

/// run game object
void GameObject::Impl::Run()
{
    for (const ComponentWeakPtr &component : mComponents)
    {
        auto componentPtr = component.lock();
        if (componentPtr)
        {
            componentPtr.get()->Run();
        }
    }
}

//==============================================================================================================================================================================

/// custom smart pointer maker, self registers into scene manager
GameObjectWeakPtr GameObject::MakePtr()
{
    GameObjectPtr gameObject = std::shared_ptr<GameObject>(new GameObject());
    GameObjectWeakPtr gameObjectWeak = SceneManager::RegisterGameObject(gameObject);
    gameObject->mImpl = std::make_unique<GameObject::Impl>(gameObjectWeak);
    return gameObjectWeak;
}

/// constructs a game object
GameObject::GameObject()
{
}

/// destroys game object
GameObject::~GameObject()
{
}

/// get component
ComponentWeakPtr GameObject::GetComponent(Type componentType) const
{
    return mImpl->GetComponent(componentType);
}

/// add a component
void GameObject::AddComponent(const ComponentWeakPtr &component)
{
    mImpl->AddComponent(component);
}

/// run game object
void GameObject::Run()
{
    mImpl->Run();
}
