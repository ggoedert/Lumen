//==============================================================================================================================================================================
/// \file
/// \brief     Mesh
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMesh.h"

using namespace Lumen;

/// Mesh::Impl class
class Mesh::Impl
{
    CLASS_NO_COPY_MOVE(Mesh::Impl);
    CLASS_PTR_UNIQUEMAKER(Mesh::Impl);
    friend class Mesh;

public:
    /// constructs a mesh
    explicit Impl() {}
};

//==============================================================================================================================================================================

/// constructs a mesh
Mesh::Mesh() : Object(Type()), mImpl(Mesh::Impl::MakeUniquePtr()) {}

/// destroys mesh
Mesh::~Mesh() noexcept {}
