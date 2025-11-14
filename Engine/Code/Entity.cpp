//==============================================================================================================================================================================
/// \file
/// \brief     entity
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEntity.h"
#include "lGeometry.h"
#include "lRenderer.h"
#include "lTransform.h"
#include "lSceneManager.h"

using namespace Lumen;

/// Entity::Impl class
class Entity::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Entity;

public:
    /// constructs a entity
    explicit Impl(EntityWeakPtr &entity, Lumen::Application &application, std::string_view name);

    /// destroys entity
    ~Impl();

    /// unregister entity from scene manager
    void Unregister() { SceneManager::UnregisterEntity(mOwner); }

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        // get transform
        Serialized::Type outTransform = {};
        mTransform->Serialize(outTransform, packed);
        if (!outTransform.empty())
        {
            Serialized::SerializeValue(out, packed, Serialized::cTransformToken, Serialized::cTransformTokenPacked, outTransform);
        }

        // get components
        Serialized::Type outComponents = {};
        for (const ComponentWeakPtr &component : mComponents)
        {
            auto componentPtr = component.lock();
            L_ASSERT(componentPtr);
            Serialized::Type outComponent = {};
            componentPtr->Serialize(outComponent, packed);
            Serialized::SerializeValue(outComponents, packed, std::string(componentPtr->Type().mName), componentPtr->Type(), outComponent);
        }
        if (!outComponents.empty())
        {
            Serialized::SerializeValue(out, packed, Serialized::cComponentsToken, Serialized::cComponentsTokenPacked, outComponents);
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
        // get transform
        Serialized::Type inTransform = {};
        Serialized::DeserializeValue(in, packed, Serialized::cTransformToken, Serialized::cTransformTokenPacked, inTransform);
        mTransform->Deserialize(inTransform, packed);

        // get components
        mComponents.clear();
        Serialized::Type inComponents = {};
        if (Serialized::DeserializeValue(in, packed, Serialized::cComponentsToken, Serialized::cComponentsTokenPacked, inComponents))
        {
            if (packed)
            {
                if (!inComponents.is_array())
                {
                    throw std::runtime_error(std::format("Unable to deserialize components, packed data must be an array"));
                }

                for (std::size_t i = 0; i + 1 < inComponents.size(); i += 2)
                {
                    auto component = AddComponent(mOwner, inComponents[i].get<Hash>());
                    component.lock()->Deserialize(inComponents[i + 1], packed);
                }
            }
            else
            {
                if (!inComponents.is_object())
                {
                    throw std::runtime_error(std::format("Unable to deserialize components, non-packed data must be an object"));
                }

                for (auto it = inComponents.begin(); it != inComponents.end(); ++it)
                {
                    auto component = AddComponent(mOwner, EncodeType(it.key().c_str()));
                    component.lock()->Deserialize(it.value(), packed);
                }
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
    [[maybe_unused]] ComponentWeakPtr AddComponent(const EntityWeakPtr &entity, Hash type);

protected:
    /// run entity
    void Run();

private:
    /// owner
    const EntityWeakPtr mOwner;

    /// application reference
    Lumen::Application &mApplication;

    /// name
    std::string mName;

    /// transform
    TransformPtr mTransform;

    /// components
    std::vector<ComponentWeakPtr> mComponents;
};

/// constructs a entity
Entity::Impl::Impl(EntityWeakPtr &entity, Lumen::Application &application, std::string_view name) :
    mOwner(entity), mApplication(application), mName(name), mTransform(Transform::MakePtr(entity)) {}

/// destroys entity
Entity::Impl::~Impl()
{
    for (const ComponentWeakPtr &component : mComponents)
    {
        SceneManager::UnregisterComponent(component);
    }
    SceneManager::UnregisterEntity(mOwner);
}

/// get component
ComponentWeakPtr Entity::Impl::Component(Hash type) const noexcept
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
ComponentWeakPtr Entity::Impl::AddComponent(const EntityWeakPtr &entity, Hash type)
{
    ComponentWeakPtr component = SceneManager::CreateComponent(mApplication.GetEngine(), entity, type);
    mComponents.push_back(component);
    return component;
}

/// run entity
void Entity::Impl::Run()
{
    RendererPtr rendererPtr;
    for (const ComponentWeakPtr &component : mComponents)
    {
        auto componentPtr = component.lock();
        L_ASSERT(componentPtr);
        if (componentPtr->Type() == Renderer::Type())
        {
            L_ASSERT(rendererPtr == nullptr);
            rendererPtr = static_pointer_cast<Renderer>(componentPtr);
        }
        componentPtr->Run();
    }
    if (rendererPtr)
    {
        rendererPtr->Render();
    }
}

//==============================================================================================================================================================================

/// constructs a entity
Entity::Entity() : Object(Type()) {}

/// destroys entity
Entity::~Entity() {}

/// custom smart pointer maker, self registers into scene manager
EntityWeakPtr Entity::MakePtr(Lumen::Application &application, std::string_view name)
{
    EntityPtr entity = EntityPtr(new Entity());
    EntityWeakPtr entityWeak = SceneManager::RegisterEntity(entity);
    entity->mImpl = std::make_unique<Entity::Impl>(entityWeak, application, name);
    return entityWeak;
}

/// serialize
void Entity::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Entity::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// get application
Application &Entity::GetApplication()
{
    return mImpl->GetApplication();
}

/// get name
std::string_view Entity::Name() const
{
    return mImpl->Name();
}

/// get transform
TransformWeakPtr Entity::Transform() const
{
    return mImpl->Transform();
}

/// get component
ComponentWeakPtr Entity::Component(Hash type) const
{
    return mImpl->Component(type);
}

/// add a component
ComponentWeakPtr Entity::AddComponent(Hash type)
{
    return mImpl->AddComponent(shared_from_this(), type);
}

/// run entity
void Entity::Run()
{
    mImpl->Run();
}
