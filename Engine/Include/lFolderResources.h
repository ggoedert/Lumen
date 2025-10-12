//==============================================================================================================================================================================
/// \file
/// \brief     FolderResources interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lAssets.h"

/// Lumen namespace
namespace Lumen
{
    /// FolderResources class
    class FolderResources : public AssetFactory
    {
    public:
        /// creates a smart pointer version of the folder resources
        static AssetFactoryPtr MakePtr();

        /// get asset infos
        [[nodiscard]] std::span<const Lumen::AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const override;

    private:
        /// constructor
        explicit FolderResources();

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
