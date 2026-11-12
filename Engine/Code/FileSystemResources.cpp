//==============================================================================================================================================================================
/// \file
/// \brief     FileSystemResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lExpected.h"
#include "lMaterial.h"
#include "lTexture.h"
#include "lMesh.h"
#include "lShader.h"
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
#ifdef EDITOR
        return FileSystem::Exists(path);
#else
        // a game build also has the assets that only exist in the Library (the builtin ones at least), their FileInfo comes from their entry
        return FileSystem::Exists(path) || AssetManager::GetAssetInfo(path).HasValue();
#endif
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

            // an asset without a source only has its Library entry
            if (!FileSystem::Exists(path))
            {
                return Expected<AssetPtr>::Unexpected(std::format("No valid Library entry for texture resource, {}", path.generic_string()));
            }

            // load the texture
            texture->Load();

            // cache the pixels, this must happen before the renderer consumes them
            AssetManager::UpdateLibrary(*texture);

            return texture;
        }
        else if (type == Mesh::Type())
        {
            // there is no mesh source format yet, a mesh comes from its Library entry
            AssetPtr mesh = Mesh::MakePtr(engine, path);
            if (!AssetManager::LoadFromLibrary(*mesh))
            {
                return Expected<AssetPtr>::Unexpected(std::format("No valid Library entry for mesh resource, {}", path.generic_string()));
            }
            return mesh;
        }
        else if (type == Shader::Type())
        {
            // there is no shader source format yet, a shader comes from its Library entry (which holds its name)
            auto shaderExpected = Shader::MakePtr(engine, path, "");
            if (!shaderExpected)
            {
                return shaderExpected;
            }
            if (!AssetManager::LoadFromLibrary(*shaderExpected.Value()))
            {
                return Expected<AssetPtr>::Unexpected(std::format("No valid Library entry for shader resource, {}", path.generic_string()));
            }
            return shaderExpected;
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
