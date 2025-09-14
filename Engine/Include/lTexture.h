//==============================================================================================================================================================================
/// \file
/// \brief     Texture interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"
#include "lEngine.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Texture);
    CLASS_WEAK_PTR_DEF(Texture);

    /// Texture class
    class Texture : public Object
    {
        CLASS_NO_COPY_MOVE(Texture);
        OBJECT_TYPEINFO;

    public:
        /// destroys texture
        virtual ~Texture() noexcept override;

        /// unregister a texture
        void Unregister();

    protected:
        /// constructs a texture
        explicit Texture(const EngineWeakPtr &engine);

        /// engine pointer
        EngineWeakPtr mEngine;

        /// engine texture id
        Engine::TextureID mTexId;

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
