//==============================================================================================================================================================================
/// \file
/// \brief     asset manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lExpected.h"
#include "lAsset.h"
#include "lFileSystem.h"
#include "lAssetInfo.h"

/// \cond
#include <functional>
/// \endcond

/// Lumen namespace
namespace Lumen
{
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(AssetFactory);

    /// AssetFactory class
    class AssetFactory
    {
        CLASS_NO_DEFAULT_CTOR(AssetFactory);
        CLASS_NO_COPY_MOVE(AssetFactory);

    public:
        /// constructs an assets factory implementation with priority
        explicit AssetFactory(float priority) : mPriority(priority) {}

        /// gets priority
        [[nodiscard]] float Priority() const noexcept { return mPriority; }

        /// check if asset exists
        [[nodiscard]] virtual bool Exists(const std::filesystem::path &path) const = 0;

        /// load asset
        [[nodiscard]] virtual Expected<AssetPtr> Load(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const = 0;

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

        /// register the asset type stored in files with the given extension
        void RegisterType(std::string_view extension, HashType type);

        /// scan assets
        void Scan();

        /// create or get assetinfo
        Expected<AssetInfoPtr> GetInfo(const std::filesystem::path &path);

        /// load asset, assets that are shareable are returned from the instance cache while someone still holds them
        Expected<AssetPtr> Load(HashType type, const std::filesystem::path &path);

        /// load asset from its asset info
        Expected<AssetPtr> Load(const AssetInfoPtr &assetInfo);

#ifdef EDITOR
        /// custom Importer type
        using Importer = std::function<bool(const AssetInfoPtr &assetInfo)>;

        /// register an importer for a specific asset type
        void RegisterImporter(std::string_view extension, Importer importer);

        /// import asset
        bool Import(const AssetInfoPtr &assetInfo);

        /// AssetChange struct
        struct AssetChange
        {
            /// change
            FileSystem::Change mChange;

            /// attributes
            FileSystem::FileAttributes mAttributes;

            /// name
            std::string mName;

            /// old name
            std::string mOldName;
        };

        /// process file changes
        void ProcessFileChanges(std::vector<FileSystem::FileChange> &&fileBatch);
#endif
    };
}
