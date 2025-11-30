//==============================================================================================================================================================================
/// \file
/// \brief     FileSystemResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lExpected.h"
#include "lMaterial.h"
#include "lFileSystemResources.h"

using namespace Lumen;

/// FileSystemResources::Impl class
class FileSystemResources::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructor
    explicit Impl() {}

    /// destructor
    ~Impl() = default;

    /// check if asset exists
    [[nodiscard]] bool Exists(const std::filesystem::path &path) const
    {
        return FileSystem::Exists(path);
    }

    /// import asset
    [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
    {
        //*WIP* FIXME temp code, we need to generalize this
        bool isMaterial = path.extension() == ".mat";
        if (isMaterial)
        {
            auto materialExpected = Material::MakePtr(path);
            if (!materialExpected)
            {
                return Expected<AssetPtr>::Unexpected(materialExpected.Error());
            }
            if (!materialExpected.Value()->Load())
            {
                return Expected<AssetPtr>::Unexpected(std::format("Unable to import material resource, {}", path.string()));
            }
            return materialExpected;
        }
        return Expected<AssetPtr>::Unexpected(std::format("Unknown resource type, {}", path.string()));
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

/// import asset
Expected<AssetPtr> FileSystemResources::Import(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
{
    return mImpl->Import(engine, type, path);
}
