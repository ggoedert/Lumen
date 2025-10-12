//==============================================================================================================================================================================
/// \file
/// \brief     Texture
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lTexture.h"

using namespace Lumen;

/// Texture::Impl class
class Texture::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Texture;

public:
    /// constructs a texture
    explicit Impl() {}
    explicit Impl(const EngineWeakPtr &engine) : mEngine(engine), mTextureId(Id::Invalid) {}

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

    /// get texture id
    Id::Type GetTextureId()
    {
        return mTextureId;
    }

    /// set texture id
    void SetTexture(Id::Type textureId)
    {
        mTextureId = textureId;
    }

    /// engine pointer
    EngineWeakPtr mEngine;

    /// engine texture id
    Id::Type mTextureId;
};

//==============================================================================================================================================================================

/// constructs a texture
Texture::Texture(const EngineWeakPtr &engine) : Object(Type()), mImpl(Texture::Impl::MakeUniquePtr(engine)) {}

/// destroys texture
Texture::~Texture()
{
    Release();
}

/// release a texture
void Texture::Release()
{
    mImpl->Release();
}

/// get texture id
Id::Type Texture::GetTextureId()
{
    return mImpl->GetTextureId();
}

/// set texture id
void Texture::SetTextureId(Id::Type textureId)
{
    mImpl->SetTexture(textureId);
}
