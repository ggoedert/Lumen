//==============================================================================================================================================================================
/// \file
/// \brief     game object
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lGameObject.h"
#include "lMeshFilter.h"
#include "lMeshRenderer.h"
#include "lTransform.h"

using namespace Lumen;

/// GameObject::Impl class
class GameObject::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class GameObject;

public:
    /// constructs a game object
    explicit Impl(GameObjectWeakPtr &gameObject, Lumen::Application &application);

    /// destroys game object
    ~Impl();

    /// get transform
    [[nodiscard]] Application &GetApplication() { return mApplication; }

    /// get transform
    [[nodiscard]] TransformWeakPtr Transform() const noexcept { return mTransform; }

    /// get component
    [[nodiscard]] ComponentWeakPtr Component(const HashType type) const noexcept;

    /// add a component
    [[maybe_unused]] ComponentWeakPtr AddComponent(const GameObjectWeakPtr &gameObject, const HashType type, const Object &params);

protected:
    /// run game object
    void Run();

private:
    /// owner
    GameObjectWeakPtr mOwner;

    /// application reference
    Lumen::Application &mApplication;

    /// transform
    TransformPtr mTransform;

    /// components
    std::vector<ComponentWeakPtr> mComponents;
};

/// constructs a game object
GameObject::Impl::Impl(GameObjectWeakPtr &gameObject, Lumen::Application &application) : mOwner(gameObject), mApplication(application), mTransform(Transform::MakePtr(gameObject)) {}

/// destroys game object
GameObject::Impl::~Impl()
{
    for (const ComponentWeakPtr &component : mComponents)
    {
        SceneManager::UnregisterComponent(component);
    }
    SceneManager::UnregisterGameObject(mOwner);
}

/// get component
ComponentWeakPtr GameObject::Impl::Component(const HashType type) const noexcept
{
    for (const ComponentWeakPtr &component : mComponents)
    {
        auto componentPtr = component.lock();
        L_ASSERT(componentPtr);
        if (componentPtr->Type() == type)
        {
            return component;
        }
    }
    return {};
}

/// add a component
ComponentWeakPtr GameObject::Impl::AddComponent(const GameObjectWeakPtr &gameObject, const HashType type, const Object &params)
{
    ComponentWeakPtr component = SceneManager::CreateComponent(mApplication.GetEngine(), gameObject, type, params);
    mComponents.push_back(component);
    return component;
}

/// run game object
void GameObject::Impl::Run()
{
    MeshRendererPtr meshRendererPtr;
    for (const ComponentWeakPtr &component : mComponents)
    {
        auto componentPtr = component.lock();
        L_ASSERT(componentPtr);
        if (componentPtr->Type() == MeshRenderer::Type())
        {
            L_ASSERT(meshRendererPtr == nullptr);
            meshRendererPtr = static_pointer_cast<MeshRenderer>(componentPtr);
        }
        componentPtr->Run();
    }
    if (meshRendererPtr)
    {
        meshRendererPtr->Render();
    }
}

//==============================================================================================================================================================================

/// constructs a game object
GameObject::GameObject() : Object(Type()) {}

/// destroys game object
GameObject::~GameObject() {}

/// custom smart pointer maker, self registers into scene manager
GameObjectWeakPtr GameObject::MakePtr(Lumen::Application &application)
{
    GameObjectPtr gameObject = std::shared_ptr<GameObject>(new GameObject());
    GameObjectWeakPtr gameObjectWeak = SceneManager::RegisterGameObject(gameObject);
    gameObject->mImpl = std::make_unique<GameObject::Impl>(gameObjectWeak, application);
    return gameObjectWeak;
}

Application &GameObject::GetApplication()
{
    return mImpl->GetApplication();
}

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
ComponentWeakPtr GameObject::AddComponent(const HashType type, const Object &params)
{
    return mImpl->AddComponent(shared_from_this(), type, params);
}

/// run game object
void GameObject::Run()
{
    mImpl->Run();
}
