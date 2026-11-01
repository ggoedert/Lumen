//==============================================================================================================================================================================
/// \file
/// \brief     Texture
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lTexture.h"
#include "lStringMap.h"
#include "lEngine.h"

/// \cond
#include <cstring>
/// \endcond

using namespace Lumen;

/// Texture::Impl class
class Texture::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Texture;

public:
    /// constructs texture
    explicit Impl(Texture &owner, const EngineWeakPtr &engine, const Info &info) : mOwner(owner), mEngine(engine), mInfo(info), mTextureId(Id::Invalid) {}

    /// destructor
    ~Impl() { Release(); }

    /// register texture name / path
    static void Register(std::string_view name, std::string_view path)
    {
        // register asset
        mAssetPaths.insert_or_assign(std::string(name), path);
    }

    /// find texture path from name
    static Expected<std::string_view> Find(std::string_view name)
    {
        // find asset
        auto it = mAssetPaths.find(name);
        if (it != mAssetPaths.end())
        {
            return it->second;
        }

        // none found
        return Expected<std::string_view>::Unexpected(std::format("Texture path for '{}' not found", name));
    }

    /// save texture
    bool Save() const
    {
        return true;
    }

    /// load texture
    bool Load() {
        return true;
    }

    /// release texture
    void Release()
    {
        if (Id::Invalid != mTextureId)
        {
            Id::Type textureId = mTextureId;
            mTextureId = Id::Invalid;
            if (auto engineLock = mEngine.lock())
            {
                engineLock->ReleaseTexture(textureId);
            }
        }
    }

    /// serialize texture, throws if not available
    void Serialize(Serialized::Type &out, bool packed) const
    {
        if (!mTextureData.Valid())
        {
            throw std::runtime_error("texture pixels are not available");
        }
        const byte *begin = mTextureData.data();
        out = Serialized::Type::object();
        out["Width"] = mInfo.mWidth;
        out["Height"] = mInfo.mHeight;
        out["Pixels"] = Serialized::Type::binary(std::vector<byte>(begin, begin + mTextureData.size()));
    }

    /// deserialize texture
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        if (!in.is_object() || !in.contains("Width") || !in.contains("Height") || !in.contains("Pixels"))
        {
            throw std::runtime_error("texture data is incomplete");
        }
        const int width = in["Width"].get<int>();
        const int height = in["Height"].get<int>();
        const auto &pixels = in["Pixels"];
        if ((width <= 0) || (height <= 0) || !pixels.is_binary())
        {
            throw std::runtime_error("texture data is invalid");
        }

        // the GPU texture was created with the size of mInfo when the asset was made, recreating it needs the shared pointer of the owner (see MakePtr)
        if ((width != mInfo.mWidth) || (height != mInfo.mHeight))
        {
            throw std::runtime_error(std::format("texture size {}x{} differs from the created size {}x{}", width, height, mInfo.mWidth, mInfo.mHeight));
        }
        const std::vector<byte> &binary = pixels.get_binary();
        if (binary.size() != static_cast<size_t>(width) * static_cast<size_t>(height) * 4)
        {
            throw std::runtime_error("texture pixel data has the wrong size");
        }

        UniqueByteArray textureData(binary.size());
        std::memcpy(textureData.data(), binary.data(), binary.size());
        PushTextureData(std::move(textureData));
    }

    /// push texture data
    void PushTextureData(UniqueByteArray textureData)
    {
        mTextureData = std::move(textureData);
    }

    /// pop texture data
    UniqueByteArray PopTextureData()
    {
        return std::move(mTextureData);
    }

    /// owner
    Texture &mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// texture info
    Info mInfo;

    /// texture data
    UniqueByteArray mTextureData;

    /// engine texture id
    Id::Type mTextureId;

    /// static map of asset names to paths
    static StringMap<std::string> mAssetPaths;
};

StringMap<std::string> Texture::Impl::mAssetPaths;

//==============================================================================================================================================================================

/// constructs texture
Texture::Texture(const EngineWeakPtr &engine, const std::filesystem::path &path, const Info &info) :
    Asset(Type(), path), mImpl(Texture::Impl::MakeUniquePtr(*this, engine, info)) {}

/// destroys texture
Texture::~Texture()
{
    Release();
}

/// creates a smart pointer version of the texture asset
AssetPtr Texture::MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path, const Info &info)
{
    auto ptr = TexturePtr(new Texture(engine, path, info));
    if (auto engineLock = engine.lock())
    {
        ptr->mImpl->mTextureId = engineLock->CreateTexture(ptr, info.mWidth, info.mHeight);
        L_ASSERT_MSG(ptr->mImpl->mTextureId != Id::Invalid, "Failed to create texture size {} {}", info.mWidth, info.mHeight);
    }
    return ptr;
}

/// register texture name / path
void Texture::Register(std::string_view name, std::string_view path)
{
    Texture::Impl::Register(name, path);
}

/// find texture path from name
Expected<std::string_view> Texture::Find(std::string_view name)
{
    return Texture::Impl::Find(name);
}

/// textures are shared between all users of the same path
bool Texture::Shareable() const
{
    return true;
}

/// textures are cached in the Library
bool Texture::SupportsLibrary() const
{
    return true;
}

/// serialize
void Texture::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Texture::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// whether the pixels are available
bool Texture::HasTextureData() const
{
    return mImpl->mTextureData.Valid();
}

/// save texture
bool Texture::Save() const
{
    return mImpl->Save();
}

/// load texture
bool Texture::Load()
{
    return mImpl->Load();
}

/// release texture
void Texture::Release()
{
    mImpl->Release();
}

/// get texture id
Id::Type Texture::GetTextureId()
{
    return mImpl->mTextureId;
}

/// push texture data
void Texture::PushTextureData(UniqueByteArray textureData)
{
    mImpl->PushTextureData(std::move(textureData));
}

/// pop texture data
UniqueByteArray Texture::PopTextureData()
{
    return mImpl->PopTextureData();
}
