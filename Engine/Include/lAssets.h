//==============================================================================================================================================================================
/// \file
/// \brief     assets interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lExpected.h"
#include "lObject.h"

#include <filesystem>

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(AssetInfo);

    /// AssetInfo class
    class AssetInfo
    {
    public:

        /// get type
        virtual [[nodiscard]] HashType Type() const = 0;

        /// get name
        virtual [[nodiscard]] std::string_view Name() const = 0;

        /// import the asset
        virtual [[nodiscard]] ObjectPtr Import() = 0;
    };

    CLASS_PTR_DEF(AssetFactory);

    /// AssetFactory class
    class AssetFactory
    {
    public:

        /// accepts a path
        virtual [[nodiscard]] bool Accepts(std::filesystem::path path) const = 0;

        /// get asset infos
        virtual [[nodiscard]] std::vector<AssetInfoPtr> GetAssetInfos() = 0;
    };

    /// Assets class
    class Assets
    {
        CLASS_NO_COPY_MOVE(Assets);

    public:
        /// constructor
        explicit Assets();

        /// destructor
        ~Assets() noexcept;

        /// register an asset factory
        void RegisterFactory(const AssetFactoryPtr &assetFactory, std::string_view extension, float priority = 0.5f);

        /// import asset
        Expected<ObjectPtr> Import(std::filesystem::path path, const HashType type, std::string_view name);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
