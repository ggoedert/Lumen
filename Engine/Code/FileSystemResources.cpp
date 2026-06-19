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
        if (type == Material::Type())
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
FileSystemResources::FileSystemResources(float priority) : AssetFactoryOld(priority), mImpl(FileSystemResources::Impl::MakeUniquePtr()) {}

/// creates a smart pointer version of the folder resources
AssetFactoryOldPtr FileSystemResources::MakePtr(float priority)
{
    return AssetFactoryOldPtr(new FileSystemResources(priority));
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
