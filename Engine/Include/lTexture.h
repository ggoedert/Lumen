//==============================================================================================================================================================================
/// \file
/// \brief     Texture interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lId.h"
#include "lUniqueByteArray.h"
#include "lAsset.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Texture);
    CLASS_WEAK_PTR_DEF(Texture);
    CLASS_WEAK_PTR_DEF(Engine);

    /// Texture class
    class Texture : public Asset
    {
        CLASS_NO_DEFAULT_CTOR(Texture);
        CLASS_NO_COPY_MOVE(Texture);
        OBJECT_TYPEINFO;

    public:
        /// texture info
        struct Info
        {
            int mWidth; int mHeight;
        };

        /// destroys texture
        ~Texture() override;

        /// creates a smart pointer version of the texture asset
        static AssetPtr MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path, const Info &info);

        /// save a texture
        bool Save() const override;

        /// load a texture
        bool Load() override;

        /// release a texture
        void Release() override;

        /// get texture id
        Id::Type GetTextureId();

        /// push texture data
        void PushTextureData(UniqueByteArray textureData);

        /// pop texture data
        UniqueByteArray PopTextureData();

    private:
        /// constructs a texture
        explicit Texture(const EngineWeakPtr &engine, const std::filesystem::path &path, const Info &info);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
