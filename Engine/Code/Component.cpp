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
    explicit Impl(std::string_view name, const EntityWeakPtr &entity) :
        mName(name), mEntity(entity) {}

    /// destroys component
    ~Impl() = default;

public:
    /// get name
    [[nodiscard]] std::string_view Name() const { return mName; }

    /// get owning entity
    [[nodiscard]] EntityWeakPtr Entity() const { return mEntity; }

private:
    /// name
    const std::string mName;

    /// owning entity
    EntityWeakPtr mEntity;
};

//==============================================================================================================================================================================

/// constructs a component with type, name, and parent. called by derived classes
Component::Component(HashType type, std::string_view name, const EntityWeakPtr &entity) :
    Object(type), mImpl(Component::Impl::MakeUniquePtr(name, entity)) {}

/// destructor
Component::~Component() = default;

/// get name
std::string_view Component::Name() const
{
    return mImpl->Name();
}

/// get owning entity
EntityWeakPtr Component::Entity() const
{
    return mImpl->Entity();
}
