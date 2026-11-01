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
    ///
    /// Assets live in two places:
    ///  - Assets/ and BuiltIn/ hold the source of truth (text, edited by hand or by the editor), every asset has a sibling .info file that holds its UUID
    ///  - Library/ holds a binary cache of the serialized asset, one file per UUID (see LibraryPath), it can be deleted at any time and is rebuilt on demand
    ///
    /// Loading goes through the Library first and falls back to the source, a successful load from the source refreshes the Library.
    /// A Library entry is only used when its format version, asset type and source (size, last write time) stamp match.
    ///
    /// Not thread safe, all calls (and the lifetime of the asset infos it hands out) are expected on the main thread.
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

        /// path of the Library entry of an asset UUID, always a plain file name in a one level fan out folder, so it is valid on every file system
        std::filesystem::path LibraryPath(UUID uuid);

        /// load the state of an asset from its Library entry, false (and the asset untouched) when there is no valid entry
        bool LoadFromLibrary(Asset &asset);

        /// load the state of an asset, from the Library when it has a valid entry, otherwise from its source (which then refreshes the Library)
        bool LoadSerialized(Asset &asset);

        /// write the Library entry of an asset, call after the source was written, best effort: a failure only means the next load uses the source
        bool UpdateLibrary(const Asset &asset);

#ifdef EDITOR
        /// custom Importer type
        using Importer = std::function<bool(const AssetInfoPtr &assetInfo)>;

        /// register an importer for a specific asset type
        void RegisterImporter(std::string_view extension, Importer importer);

        /// import asset, runs the registered importer for the extension, without one it drops a stale Library entry so the next load rebuilds it
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
