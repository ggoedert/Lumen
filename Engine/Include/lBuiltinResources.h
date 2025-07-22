//==============================================================================================================================================================================
/// \file
/// \brief     BuiltinResources interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lAssets.h"

/// Lumen namespace
namespace Lumen
{
    /// DefaultResources class
    class DefaultResources : public AssetFactory
    {
    public:
        /// creates a smart pointer version of the default resources
        static AssetFactoryPtr MakePtr();

        /// accepts a path
        [[nodiscard]] bool Accepts(std::filesystem::path path) const override;

        /// get asset infos
        [[nodiscard]] std::vector<Lumen::AssetInfoPtr> GetAssetInfos() override;
    };
}
