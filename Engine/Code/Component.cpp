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
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Component;

public:
    /// constructs a component
    explicit Impl(std::string_view name, const GameObjectWeakPtr &gameObject) :
        mName(name), mGameObject(gameObject) {}

    /// destroys component
    ~Impl() = default;

public:
    /// get name
    [[nodiscard]] std::string_view Name() const noexcept { return mName; }

    /// get owning game object
    [[nodiscard]] GameObjectWeakPtr GameObject() const { return mGameObject; }

private:
    /// start component
    void Start() {}

    /// name
    const std::string mName;

    /// owning game object
    GameObjectWeakPtr mGameObject;
};

//==============================================================================================================================================================================

/// constructs a component with type, name, and parent. called by derived classes
Component::Component(const HashType type, std::string_view name, const GameObjectWeakPtr &gameObject) :
    Object(type), mImpl(Component::Impl::MakeUniquePtr(name, gameObject)) {}

/// destructor
Component::~Component() noexcept = default;

/// get name
std::string_view Component::Name() const noexcept
{
    return mImpl->Name();
}

/// get owning game object
GameObjectWeakPtr Component::GameObject() const
{
    return mImpl->GameObject();
}

/// start component
void Component::Start()
{
    mImpl->Start();
}
