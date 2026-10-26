//==============================================================================================================================================================================
/// \file
/// \brief     FileSystemResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lExpected.h"
#include "lMaterial.h"
#include "lFileSystem.h"
#include "lFileSystemResources.h"

using namespace Lumen;

/// FileSystemResources::Impl class
class FileSystemResources::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructor
    explicit Impl() = default;

    /// destructor
    ~Impl() = default;

    /// check if asset exists
    [[nodiscard]] bool Exists(const std::filesystem::path &path) const
    {
        return FileSystem::Exists(path);
    }

    /// load asset
    [[nodiscard]] Expected<AssetPtr> Load(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
    {
        if (type == Material::Type())
        {
            auto materialExpected = Material::MakePtr(path);
            if (!materialExpected)
            {
                return Expected<AssetPtr>::Unexpected(materialExpected.Error());
            }
            if (!materialExpected.Value()->Load())
            {
                return Expected<AssetPtr>::Unexpected(std::format("Unable to load material resource, {}", path.generic_string()));
            }
            return materialExpected;
        }
        else if (type == Texture::Type())
        {
            Texture::Info textureInfo { 64, 64 };
            TexturePtr texture = static_pointer_cast<Texture>(Texture::MakePtr(engine, path, textureInfo));
            texture->Load();

            UniqueByteArray textureData(textureInfo.mWidth * textureInfo.mHeight * 4);
            int textureDataPitch = textureInfo.mWidth * 4;
            for (int y = 0; y < textureInfo.mHeight; y++)
            {
                for (int x = 0; x < textureInfo.mWidth; x++)
                {
                    if ((x < (textureInfo.mWidth >> 1)) == (y < (textureInfo.mHeight >> 1)))
                    {
                        textureData[y * textureDataPitch + 4 * x + 0] = 198;
                        textureData[y * textureDataPitch + 4 * x + 1] = 197;
                        textureData[y * textureDataPitch + 4 * x + 2] = 198;
                    }
                    else
                    {
                        textureData[y * textureDataPitch + 4 * x + 0] = 156;
                        textureData[y * textureDataPitch + 4 * x + 1] = 158;
                        textureData[y * textureDataPitch + 4 * x + 2] = 156;
                    }
                    textureData[y * textureDataPitch + 4 * x + 3] = 255;
                }
            }
            texture->PushTextureData(std::move(textureData));

            return texture;
        }
        return Expected<AssetPtr>::Unexpected(std::format("Unknown resource type, {}", path.generic_string()));
    }
};

//==============================================================================================================================================================================

/// constructs folder resources
FileSystemResources::FileSystemResources(float priority) : AssetFactory(priority), mImpl(FileSystemResources::Impl::MakeUniquePtr()) {}

/// creates a smart pointer version of the folder resources
AssetFactoryPtr FileSystemResources::MakePtr(float priority)
{
    return AssetFactoryPtr(new FileSystemResources(priority));
}

/// check if asset exists
[[nodiscard]] bool FileSystemResources::Exists(const std::filesystem::path &path) const
{
    return mImpl->Exists(path);
}

/// load asset
Expected<AssetPtr> FileSystemResources::Load(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
{
    return mImpl->Load(engine, type, path);
}
