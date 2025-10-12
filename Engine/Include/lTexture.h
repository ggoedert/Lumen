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

    /// Texture class
    class Texture : public Object
    {
        CLASS_NO_COPY_MOVE(Texture);
        OBJECT_TYPEINFO;

    public:
        /// destroys texture
        ~Texture() override;

        /// release a texture
        void Release();

        /// get texture data
        virtual void GetTextureData(byte *data, int pitch) = 0;

        /// get texture id
        Id::Type GetTextureId();

        /// set texture id
        void SetTextureId(Id::Type textureId);

    protected:
        /// constructs a texture
        explicit Texture(const EngineWeakPtr &engine);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
