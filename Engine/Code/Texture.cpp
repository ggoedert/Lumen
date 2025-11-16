//==============================================================================================================================================================================
/// \file
/// \brief     Texture
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lTexture.h"
#include "lEngine.h"

using namespace Lumen;

/// Texture::Impl class
class Texture::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Texture;

public:
    /// constructs a texture
    explicit Impl(const EngineWeakPtr &engine, const Info &info) : mEngine(engine), mInfo(info), mTextureId(Id::Invalid) {}

    /// save a texture
    bool Save() const
    {
        return true;
    }

    /// load a texture
    bool Load() {
        return true;
    }

    /// release a texture
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
    TextureWeakPtr mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// texture info
    Info mInfo;

    /// texture data
    UniqueByteArray mTextureData;

    /// engine texture id
    Id::Type mTextureId;
};

//==============================================================================================================================================================================

/// constructs a texture
Texture::Texture(const EngineWeakPtr &engine, const std::filesystem::path &path, const Info &info) :
    Asset(Type(), path), mImpl(Texture::Impl::MakeUniquePtr(engine, info)) {}

/// destroys texture
Texture::~Texture()
{
    Release();
}

/// creates a smart pointer version of the texture asset
AssetPtr Texture::MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path, const Info &info)
{
    auto ptr = TexturePtr(new Texture(engine, path, info));
    ptr->mImpl->mOwner = ptr;
    if (auto engineLock = engine.lock())
    {
        ptr->mImpl->mTextureId = engineLock->CreateTexture(ptr, info.mWidth, info.mHeight);
        L_ASSERT_MSG(ptr->mImpl->mTextureId != Id::Invalid, "Failed to create texture size {} {}", info.mWidth, info.mHeight);
    }
    return ptr;
}

/// save a texture
bool Texture::Save() const
{
    return mImpl->Save();
}

/// load a texture
bool Texture::Load()
{
    return mImpl->Load();
}

/// release a texture
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
