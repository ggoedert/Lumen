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
};

//==============================================================================================================================================================================

/// constructs a texture
Texture::Texture(const EngineWeakPtr &engine) : Object(Type()), mEngine(engine), mTexId(Engine::InvalidId), mImpl(Texture::Impl::MakeUniquePtr()) {}

/// destroys texture
Texture::~Texture() noexcept
{
    Release();
}

/// release a texture
void Texture::Release()
{
    if (Engine::InvalidId != mTexId)
    {
        Engine::IdType texId = mTexId;
        mTexId = Engine::InvalidId;
        if (auto engineLock = mEngine.lock())
        {
            engineLock->ReleaseTexture(texId);
        }
    }
}
