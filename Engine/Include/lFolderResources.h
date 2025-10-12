//==============================================================================================================================================================================
/// \file
/// \brief     FolderResources interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lAssets.h"

/// Lumen namespace
namespace Lumen
{
    /// FolderResources class
    class FolderResources : public AssetFactory
    {
        CLASS_NO_DEFAULT_CTOR(FolderResources);

    public:
        /// creates a smart pointer version of the folder resources
        static AssetFactoryPtr MakePtr(float priority);

        /// get asset infos
        [[nodiscard]] std::span<const Lumen::AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const override;

    private:
        /// constructor
        explicit FolderResources(float priority);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
