//==============================================================================================================================================================================
/// \file
/// \brief     Component
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lComponent.h"

using namespace Lumen;

/// Component::Impl class
class Component::Impl
{
    CLASS_NO_DEFAULT_CTOR(Component::Impl);
    CLASS_NO_COPY_MOVE(Component::Impl);
    CLASS_PTR_UNIQUEMAKER(Component::Impl);
    friend class Component;

public:
    /// constructs a component
    Impl(Type componentType, const std::string &componentName, const GameObjectWeakPtr &gameObject) :
        mComponentType(componentType), mComponentName(componentName), mGameObject(gameObject) {}

    /// destroys component
    ~Impl() = default;

public:
    /// get component type
    [[nodiscard]] Type ComponentType() const noexcept { return mComponentType; }

    /// get component name
    [[nodiscard]] std::string ComponentName() const noexcept { return mComponentName; }

    /// get owning game object
    [[nodiscard]] GameObjectWeakPtr GetGameObject() const { return mGameObject; }

private:
    /// start component
    void Start() {}

    /// component type
    const Type mComponentType;

    /// component name
    const std::string &mComponentName;

    /// owning game object
    GameObjectWeakPtr mGameObject;
};

//==============================================================================================================================================================================

/// constructs a component with type, name, and parent. called by derived classes
Component::Component(Type componentType, const std::string &componentName, const GameObjectWeakPtr &gameObject) :
    Object(), mImpl(Component::Impl::MakeUniquePtr(componentType, componentName, gameObject)) {}

/// destructor
Component::~Component() noexcept = default;

/// get component type
Type Component::ComponentType() const noexcept
{
    return mImpl->ComponentType();
}

/// get component name
std::string Component::ComponentName() const noexcept
{
    return mImpl->ComponentName();
}

/// get owning game object
GameObjectWeakPtr Component::GetGameObject() const
{
    return mImpl->GetGameObject();
}

/// start component
void Component::Start()
{
    mImpl->Start();
}
