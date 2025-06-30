//==============================================================================================================================================================================
/// \file
/// \brief     Resources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lResources.h"
#include "lMesh.h"

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

/// destructor
Resources::~Resources() {}

/// import resource
ObjectPtr Resources::Import(std::string_view path, const HashType type, std::optional<std::string_view> name)
{
    // WIP for now, only a test mesh construction
    return Mesh::MakePtr();
}
