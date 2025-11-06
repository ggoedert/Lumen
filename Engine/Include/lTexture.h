//==============================================================================================================================================================================
/// \file
/// \brief     Texture interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lId.h"
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
        static AssetPtr MakePtr(EngineWeakPtr &engine, std::string_view name, const std::filesystem::path &path, const Info &info);

        /// release a texture
        void Release();

        /// get texture id
        Id::Type GetTextureId();

        /// get texture data
        void GetTextureData(byte *data, int pitch);

    private:
        /// constructs a texture
        explicit Texture(const EngineWeakPtr &engine, std::string_view name, const std::filesystem::path &path, const Info &info);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
