//==============================================================================================================================================================================
/// \file
/// \brief     asset manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
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

        /// get path
        [[nodiscard]] virtual std::string_view Path() const = 0;

        /// get name
        [[nodiscard]] virtual std::string_view Name() const = 0;

        /// import the asset
        [[nodiscard]] virtual Lumen::Expected<ObjectPtr> Import(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name) = 0;
    };

    CLASS_PTR_DEF(AssetFactory);

    /// AssetFactory class
    class AssetFactory
    {
        CLASS_NO_DEFAULT_CTOR(AssetFactory);

    public:
        /// constructs an assets factory implementation with priority
        explicit AssetFactory(float priority) : mPriority(priority) {}

        /// gets priority
        [[nodiscard]] float Priority() const noexcept { return mPriority; }

        /// get asset infos
        [[nodiscard]] virtual std::vector<AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const = 0;

    private:
        /// priority
        float mPriority;
    };

    /// AssetManager namespace
    namespace AssetManager
    {
        /// initialize asset manager namespace
        void Initialize(const EngineWeakPtr &engine);

        /// shutdown asset manager namespace
        void Shutdown();

        /// register an asset factory
        void RegisterFactory(const AssetFactoryPtr &assetFactory);

        /// register an asset info
        void RegisterAssetInfo(const std::filesystem::path &path, HashType type, std::string_view name, AssetInfoPtr &assetInfoPtr, float priority);

        /// import asset
        Expected<ObjectPtr> Import(const std::filesystem::path &path, HashType type, std::string_view name);

        /// import asset from name
        Expected<ObjectPtr> GlobalImport(HashType type, std::string_view name);
    };
}
