//==============================================================================================================================================================================
/// \file
/// \brief     assets interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lExpected.h"
#include "lObject.h"
#include "lFileSystem.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(AssetInfo);

    /// AssetInfo class
    class AssetInfo
    {
    public:

        /// get type
        [[nodiscard]] virtual HashType Type() const = 0;

        /// get name
        [[nodiscard]] virtual std::string_view Name() const = 0;

        /// import the asset
        [[nodiscard]] virtual Lumen::Expected<ObjectPtr> Import(EngineWeakPtr &engine) = 0;
    };

    CLASS_PTR_DEF(AssetFactory);

    /// AssetFactory class
    class AssetFactory
    {
    public:

        /// get asset infos
        [[nodiscard]] virtual std::span<const AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const = 0;
    };

    /// Assets namespace
    namespace Assets
    {
        /// initialize assets namespace
        void Initialize(const EngineWeakPtr &engine);

        /// shutdown assets namespace
        void Shutdown();

        /// register an asset info (for built-in assets)
        void RegisterAssetInfo(HashType hashType, std::string_view name, AssetInfoPtr &assetInfoPtr);

        /// register an asset factory
        void RegisterFactory(const AssetFactoryPtr &assetFactory, float priority = 0.5f);

        /// import asset
        Expected<ObjectPtr> Import(std::optional<std::filesystem::path> path, const HashType type, std::string_view name);
    };
}
