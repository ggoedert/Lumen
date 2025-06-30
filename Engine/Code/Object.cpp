//==============================================================================================================================================================================
/// \file
/// \brief     Object
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lObject.h"

using namespace Lumen;

/// Object::Impl class
class Object::Impl
{
    CLASS_NO_DEFAULT_CTOR(Object::Impl);
    CLASS_NO_COPY_MOVE(Object::Impl);
    CLASS_PTR_UNIQUEMAKER(Object::Impl);
    friend class Object;

public:
    /// constructs a component
    explicit Impl(const HashType type) : mType(type) {}

    /// destroys component
    ~Impl() = default;

public:
    /// get type
    [[nodiscard]] HashType Type() const noexcept { return mType; }

private:
    /// type
    const HashType mType;
};

//==============================================================================================================================================================================

/// constructs an object with type. called by derived classes
Object::Object(const HashType type) : mImpl(Object::Impl::MakeUniquePtr(type)) {}

/// destructor
Object::~Object() noexcept = default;

/// get type
HashType Object::Type() const noexcept
{
    return mImpl->Type();
}
