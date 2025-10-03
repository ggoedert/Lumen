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
        [[nodiscard]] const std::vector<Lumen::AssetInfoPtr> &GetAssetInfos() const override;

    private:
        /// constructor
        explicit DefaultResources();

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };

    /// BuiltinExtra class
    class BuiltinExtra : public AssetFactory
    {
    public:
        /// creates a smart pointer version of the builtin extra
        static AssetFactoryPtr MakePtr();

        /// accepts a path
        [[nodiscard]] bool Accepts(std::filesystem::path path) const override;

        /// get asset infos
        [[nodiscard]] const std::vector<Lumen::AssetInfoPtr> &GetAssetInfos() const override;

    private:
        /// constructor
        explicit BuiltinExtra();

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };

}
