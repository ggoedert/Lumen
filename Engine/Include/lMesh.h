//==============================================================================================================================================================================
/// \file
/// \brief     Mesh interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"
#include "lResources.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Mesh);

    /// Mesh class
    class Mesh : public Object
    {
        CLASS_NO_COPY_MOVE(Mesh);
        CLASS_PTR_MAKER(Mesh);
        OBJECT_TYPEINFO;

    public:
        /// destroys mesh
        ~Mesh() noexcept override;

    private:
        /// constructs a mesh
        explicit Mesh();

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
