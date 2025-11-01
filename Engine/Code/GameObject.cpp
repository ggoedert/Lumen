//==============================================================================================================================================================================
/// \file
/// \brief     game object
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lGameObject.h"
#include "lMeshFilter.h"
#include "lMeshRenderer.h"
#include "lTransform.h"
#include "lSceneManager.h"

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
    explicit Impl(GameObjectWeakPtr &gameObject, Lumen::Application &application, std::string_view name);

    /// destroys game object
    ~Impl();

    /// unregister game object from scene manager
    void Unregister() { SceneManager::UnregisterGameObject(mOwner); }

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        // tokens
        const std::string &transformToken = packed ? Serialized::cTransformTokenPacked : Serialized::cTransformToken;
        const std::string &componentsToken = packed ? Serialized::cComponentsTokenPacked : Serialized::cComponentsToken;

        // get transform
        Serialized::Type outTransform = {};
        mTransform->Serialize(outTransform, packed);
        if (!outTransform.empty())
        {
            out[transformToken] = outTransform;
        }

        // get components
        Serialized::Type outComponents = {};
        for (const ComponentWeakPtr &component : mComponents)
        {
            auto componentPtr = component.lock();
            L_ASSERT(componentPtr);
#ifdef TYPEINFO
            std::string componentTypeKey = packed ? Base64Encode(componentPtr->Type()) : std::string(componentPtr->Type().mName);
#else
            std::string componentTypeKey = Base64Encode(componentPtr->Type());
#endif
            componentPtr->Serialize(outComponents[componentTypeKey], packed);
        }
        if (!outComponents.empty())
        {
            out[componentsToken] = outComponents;
        }

        // if empty, set to object
        if (out.empty())
        {
            out = Serialized::Type::object();
        }
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        // tokens
        const std::string &transformToken = packed ? Serialized::cTransformTokenPacked : Serialized::cTransformToken;
        const std::string &componentsToken = packed ? Serialized::cComponentsTokenPacked : Serialized::cComponentsToken;

        // set transform
        Serialized::Type transformIn = {};
        if (in.contains(transformToken))
        {
            transformIn = in[transformToken];
        }
        mTransform->Deserialize(transformIn, packed);

        // set components
        mComponents.clear();
        if (in.contains(componentsToken))
        {
            for (auto &inComponent : in[componentsToken].items())
            {
                const std::string &componentTypeName = inComponent.key();
#ifdef TYPEINFO
                Hash componentType = packed ? Base64Decode(componentTypeName.c_str()) : EncodeType(componentTypeName.c_str());
#else
                Hash componentType = Base64Decode(componentTypeName.c_str());
#endif
                auto component = AddComponent(mOwner, componentType);
                component.lock()->Deserialize(inComponent.value(), packed);
            }
        }
    }

    /// get transform
    [[nodiscard]] Application &GetApplication() { return mApplication; }

    /// get name
    [[nodiscard]] std::string_view Name() const { return mName; }

    /// get transform
    [[nodiscard]] TransformWeakPtr Transform() const noexcept { return mTransform; }

    /// get component
    [[nodiscard]] ComponentWeakPtr Component(Hash type) const noexcept;

    /// add a component
    [[maybe_unused]] ComponentWeakPtr AddComponent(const GameObjectWeakPtr &gameObject, Hash type);

protected:
    /// run game object
    void Run();

private:
    /// owner
    const GameObjectWeakPtr mOwner;

    /// application reference
    Lumen::Application &mApplication;

    /// name
    std::string mName;

    /// transform
    TransformPtr mTransform;

    /// components
    std::vector<ComponentWeakPtr> mComponents;
};

/// constructs a game object
GameObject::Impl::Impl(GameObjectWeakPtr &gameObject, Lumen::Application &application, std::string_view name) :
    mOwner(gameObject), mApplication(application), mName(name), mTransform(Transform::MakePtr(gameObject)) {}

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
ComponentWeakPtr GameObject::Impl::Component(Hash type) const noexcept
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
ComponentWeakPtr GameObject::Impl::AddComponent(const GameObjectWeakPtr &gameObject, Hash type)
{
    ComponentWeakPtr component = SceneManager::CreateComponent(mApplication.GetEngine(), gameObject, type);
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
GameObjectWeakPtr GameObject::MakePtr(Lumen::Application &application, std::string_view name)
{
    GameObjectPtr gameObject = GameObjectPtr(new GameObject());
    GameObjectWeakPtr gameObjectWeak = SceneManager::RegisterGameObject(gameObject);
    gameObject->mImpl = std::make_unique<GameObject::Impl>(gameObjectWeak, application, name);
    return gameObjectWeak;
}

/// serialize
void GameObject::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void GameObject::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// get application
Application &GameObject::GetApplication()
{
    return mImpl->GetApplication();
}

/// get name
std::string_view GameObject::Name() const
{
    return mImpl->Name();
}

/// get transform
TransformWeakPtr GameObject::Transform() const
{
    return mImpl->Transform();
}

/// get component
ComponentWeakPtr GameObject::Component(Hash type) const
{
    return mImpl->Component(type);
}

/// add a component
ComponentWeakPtr GameObject::AddComponent(Hash type)
{
    return mImpl->AddComponent(shared_from_this(), type);
}

/// run game object
void GameObject::Run()
{
    mImpl->Run();
}
