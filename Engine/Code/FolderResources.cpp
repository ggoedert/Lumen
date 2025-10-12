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
    [[nodiscard]] std::span<const AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const
    {
        if (path == mBasePath)
        {
            return mAssetInfos;
        }
        return {};
    }

private:
    /// asset infos
    std::vector<AssetInfoPtr> mAssetInfos;

    /// base path
    std::filesystem::path mBasePath = "Library/lumen folder resources";
};

//==============================================================================================================================================================================

/// constructs folder resources
FolderResources::FolderResources() : mImpl(FolderResources::Impl::MakeUniquePtr()) {}

/// creates a smart pointer version of the folder resources
AssetFactoryPtr FolderResources::MakePtr()
{
    return AssetFactoryPtr(new FolderResources());
}

/// get asset infos
std::span<const AssetInfoPtr> FolderResources::GetAssetInfos(const std::filesystem::path &path) const
{
    return mImpl->GetAssetInfos(path);
}
