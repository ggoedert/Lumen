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

    /// get asset infos
    [[nodiscard]] std::vector<AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const
    {
        std::vector<AssetInfoPtr> result;
        for (auto assetInfo : mAssetInfos)
        {
            if (assetInfo->Path() == path)
            {
                result.push_back(assetInfo);
            }
        }
        return result;
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

/// get asset infos
std::vector<AssetInfoPtr> FolderResources::GetAssetInfos(const std::filesystem::path &path) const
{
    return mImpl->GetAssetInfos(path);
}
