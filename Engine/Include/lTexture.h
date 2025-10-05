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

        /// release a texture
        void Release();

        /// get texture data
        virtual void GetTextureData(byte *data, int pitch) = 0;

    protected:
        /// constructs a texture
        explicit Texture(const EngineWeakPtr &engine);

        /// engine pointer
        EngineWeakPtr mEngine;

        /// engine texture id
        Engine::IdType mTexId;

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
