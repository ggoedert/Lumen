//==============================================================================================================================================================================
/// \file
/// \brief     FolderResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFolderResources.h"

using namespace Lumen;

/// FolderResources::Impl class
class FolderResources::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructor
    explicit Impl()
    {
        // @WIP@ add default resources from folder here
    }

    /// destructor
    ~Impl() = default;

    /// import asset
    [[nodiscard]] AssetPtr Import(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
    {
        for (auto assetInfo : mAssetInfos)
        {
            if ((assetInfo->Type() == type) && (assetInfo->Path() == path))
            {
                auto assetExpected = assetInfo->Import(engine);
                if (assetExpected)
                {
                    return assetExpected.Value();
                }
                else
                {
                    Lumen::DebugLog::Error("FolderResources import, {}", assetExpected.Error());
                }
            }
        }
        return nullptr;
    }

private:
    /// asset infos
    std::vector<AssetInfoPtr> mAssetInfos;
};

//==============================================================================================================================================================================

/// constructs folder resources
FolderResources::FolderResources(float priority) : AssetFactory(priority), mImpl(FolderResources::Impl::MakeUniquePtr()) {}

/// creates a smart pointer version of the folder resources
AssetFactoryPtr FolderResources::MakePtr(float priority)
{
    return AssetFactoryPtr(new FolderResources(priority));
}

/// import asset
AssetPtr FolderResources::Import(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
{
    return mImpl->Import(engine, type, path);
}
