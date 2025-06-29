//==============================================================================================================================================================================
/// \file
/// \brief     Resources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lResources.h"

using namespace Lumen;

/// Resources::Impl class
class Resources::Impl
{
    CLASS_NO_COPY_MOVE(Resources::Impl);
    CLASS_PTR_UNIQUEMAKER(Resources::Impl);
    friend class Resources;

public:
    /// constructs a resources
    explicit Impl() {}

    /// destroys behavior
    ~Impl() = default;
};

//==============================================================================================================================================================================

/// constructor
Resources::Resources() : mImpl(Resources::Impl::MakeUniquePtr()) {}

Resources::~Resources() {}

/// import resource
ObjectPtr Resources::Import(const std::string &path, const HashType type, std::optional<const std::string> name)
{
    // Implementation of resource import logic goes here
    // For now, we return a null pointer as a placeholder
    return ObjectPtr();
}
