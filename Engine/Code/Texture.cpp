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

    /// get texture data
    void GetTextureData(byte *data, int pitch)
    {
        const std::filesystem::path &path = mOwner.lock()->Path();
        if (path.string() == "lumen_builtin_extra/Assets/Texture2D/Default-Checker-Gray.png") //@REVIEW@ FIXME temp hack
        {
            for (int y = 0; y < mInfo.mHeight; y++)
            {
                for (int x = 0; x < mInfo.mWidth; x++)
                {
                    if ((x < (mInfo.mWidth >> 1)) == (y < (mInfo.mHeight >> 1)))
                    {
                        data[y * pitch + 4 * x + 0] = 198;
                        data[y * pitch + 4 * x + 1] = 197;
                        data[y * pitch + 4 * x + 2] = 198;
                    }
                    else
                    {
                        data[y * pitch + 4 * x + 0] = 156;
                        data[y * pitch + 4 * x + 1] = 158;
                        data[y * pitch + 4 * x + 2] = 156;
                    }
                    data[y * pitch + 4 * x + 3] = 255;
                }
            }
        }
    }

    /// owner
    TextureWeakPtr mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// texture info
    Info mInfo;

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

/// get texture data
void Texture::GetTextureData(byte *data, int pitch)
{
    mImpl->GetTextureData(data, pitch);
}
