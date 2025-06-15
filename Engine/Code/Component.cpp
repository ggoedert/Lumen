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
    explicit Impl(const HashType type, const std::string &name, const GameObjectWeakPtr &gameObject) :
        mType(type), mName(name), mGameObject(gameObject) {}

    /// destroys component
    ~Impl() = default;

public:
    /// get type
    [[nodiscard]] HashType Type() const noexcept { return mType; }

    /// get name
    [[nodiscard]] std::string Name() const noexcept { return mName; }

    /// get owning game object
    [[nodiscard]] GameObjectWeakPtr GameObject() const { return mGameObject; }

private:
    /// start component
    void Start() {}

    /// type
    const HashType mType;

    /// name
    const std::string &mName;

    /// owning game object
    GameObjectWeakPtr mGameObject;
};

//==============================================================================================================================================================================

/// constructs a component with type, name, and parent. called by derived classes
Component::Component(const HashType type, const std::string &name, const GameObjectWeakPtr &gameObject) :
    Object(), mImpl(Component::Impl::MakeUniquePtr(type, name, gameObject)) {}

/// destructor
Component::~Component() noexcept = default;

/// get type
HashType Component::Type() const noexcept
{
    return mImpl->Type();
}

/// get name
std::string Component::Name() const noexcept
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
