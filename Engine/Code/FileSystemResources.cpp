//==============================================================================================================================================================================
/// \file
/// \brief     FileSystemResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lExpected.h"
#include "lMaterial.h"
#include "lTexture.h"
#include "lScene.h"
#include "lEngine.h"
#include "lFileSystem.h"
#include "lAssetManager.h"
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
        else if (type == Scene::Type())
        {
            // a scene needs the application it is loaded into
            auto engineLock = engine.lock();
            auto applicationLock = engineLock ? engineLock->GetApplication().lock() : nullptr;
            if (!applicationLock)
            {
                return Expected<AssetPtr>::Unexpected(std::format("Unable to load scene resource, no application, {}", path.generic_string()));
            }
            ScenePtr scene = Scene::MakePtr(*applicationLock, path);
            if (!scene->Load())
            {
                return Expected<AssetPtr>::Unexpected(std::format("Unable to load scene resource, {}", path.generic_string()));
            }
            return scene;
        }
        else if (type == Texture::Type())
        {
            TexturePtr texture = static_pointer_cast<Texture>(Texture::MakePtr(engine, path));

            // the pixels come from the Library when it has a valid entry for this texture
            if (AssetManager::LoadFromLibrary(*texture))
            {
                return texture;
            }

            // load the texture
            texture->Load();

            // cache the pixels, this must happen before the renderer consumes them
            AssetManager::UpdateLibrary(*texture);

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
