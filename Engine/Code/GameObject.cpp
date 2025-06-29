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
    explicit Impl(GameObjectWeakPtr &gameObject);

    /// destroys game object
    ~Impl();

    /// get transform
    [[nodiscard]] TransformWeakPtr Transform() const noexcept { return mTransform; }

    /// get component
    [[nodiscard]] ComponentWeakPtr Component(const HashType type) const noexcept;

    /// add a component
    [[nodiscard]] ComponentWeakPtr AddComponent(const GameObjectWeakPtr &gameObject, const HashType type, const std::any &params);

protected:
    /// run game object
    void Run();

private:
    /// interface
    GameObjectWeakPtr mInterface;

    /// transform
    TransformPtr mTransform;

    /// components
    std::vector<ComponentWeakPtr> mComponents;
};

/// constructs a game object
GameObject::Impl::Impl(GameObjectWeakPtr &gameObject) : mInterface(gameObject), mTransform(Transform::MakePtr(gameObject)) {}

/// destroys game object
GameObject::Impl::~Impl() noexcept
{
    for (const ComponentWeakPtr component : mComponents)
    {
        SceneManager::UnregisterComponent(component);
    }
    SceneManager::UnregisterGameObject(mInterface);
}

/// get component
ComponentWeakPtr GameObject::Impl::Component(const HashType type) const noexcept
{
    for (const ComponentWeakPtr &component : mComponents)
    {
        auto componentPtr = component.lock();
        LUMEN_ASSERT(componentPtr);
        if (componentPtr->Type() == type)
        {
            return component;
        }
    }
    return {};
}

/// add a component
ComponentWeakPtr GameObject::Impl::AddComponent(const GameObjectWeakPtr &gameObject, const HashType type, const std::any &params)
{
    ComponentWeakPtr component = Lumen::SceneManager::CreateComponent(gameObject, type, params);
    mComponents.push_back(component);
    return component;
}

/// run game object
void GameObject::Impl::Run()
{
    for (const ComponentWeakPtr &component : mComponents)
    {
        auto componentPtr = component.lock();
        LUMEN_ASSERT(componentPtr);
        componentPtr->Run();
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
GameObject::GameObject() {}

/// destroys game object
GameObject::~GameObject() noexcept {}

/// get transform
TransformWeakPtr GameObject::Transform() const
{
    return mImpl->Transform();
}

/// get component
ComponentWeakPtr GameObject::Component(const HashType type) const
{
    return mImpl->Component(type);
}

/// add a component
ComponentWeakPtr GameObject::AddComponent(const HashType type, const std::any &params)
{
    return mImpl->AddComponent(shared_from_this(), type, params);
}

/// run game object
void GameObject::Run()
{
    mImpl->Run();
}
