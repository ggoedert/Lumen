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
Texture::Texture(const EngineWeakPtr &engine) : Object(Type()), mEngine(engine), mTexId(Engine::InvalidTextureID), mImpl(Texture::Impl::MakeUniquePtr()) {}

/// unregister a texture
void Texture::Unregister()
{
    Engine::TextureID texId = mTexId;
    mTexId = Engine::InvalidTextureID;
    if (auto engineLock = mEngine.lock())
    {
        engineLock->UnregisterTexture(texId);
    }
}

/// destroys texture
Texture::~Texture() noexcept {}
