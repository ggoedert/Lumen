//==============================================================================================================================================================================
/// \file
/// \brief     BuiltinResources interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lAssetManager.h"

/// Lumen namespace
namespace Lumen
{
    /// BuiltinResources class
    class BuiltinResources : public AssetFactory
    {
        CLASS_NO_DEFAULT_CTOR(BuiltinResources);

    public:
        /// creates a smart pointer version of the default resources
        static AssetFactoryPtr MakePtr(float priority);

        /// get asset infos
        [[nodiscard]] std::vector<Lumen::AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const override;

    private:
        /// constructor
        explicit BuiltinResources(float priority);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
