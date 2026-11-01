//==============================================================================================================================================================================
/// \file
/// \brief     Texture interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lId.h"
#include "lUniqueByteArray.h"
#include "lExpected.h"
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

        /// register texture name / path
        static void Register(std::string_view name, std::string_view path);

        /// find texture path from name
        static Expected<std::string_view> Find(std::string_view name);

        /// textures are shared between all users of the same path
        [[nodiscard]] bool Shareable() const override;

        /// textures are cached in the Library
        [[nodiscard]] bool SupportsLibrary() const override;

        /// serialize texture, throws if not available
        void Serialize(Serialized::Type &out, bool packed) const override;

        /// deserialize texture
        void Deserialize(const Serialized::Type &in, bool packed) override;

        /// whether the pixels are available (not yet consumed by the upload to the GPU)
        [[nodiscard]] bool HasTextureData() const;

        /// save texture
        bool Save() const override;

        /// load texture
        bool Load() override;

        /// release texture
        void Release() override;

        /// get texture id
        Id::Type GetTextureId();

        /// push texture data
        void PushTextureData(UniqueByteArray textureData);

        /// pop texture data
        UniqueByteArray PopTextureData();

    private:
        /// constructs texture
        explicit Texture(const EngineWeakPtr &engine, const std::filesystem::path &path, const Info &info);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
