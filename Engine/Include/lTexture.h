//==============================================================================================================================================================================
/// \file
/// \brief     Texture interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Texture);

    /// Texture class
    class Texture : public Object
    {
        CLASS_NO_COPY_MOVE(Texture);
        OBJECT_TYPEINFO;

    public:
        /// destroys texture
        ~Texture() noexcept override;

    protected:
        /// constructs a texture
        explicit Texture();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
