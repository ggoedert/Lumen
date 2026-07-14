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

/// Lumen namespace
namespace Lumen
{
    /// AssetManager namespace
    namespace AssetManager
    {
        /// initialize asset manager namespace
        void Initialize(const EngineWeakPtr &engine);

        /// shutdown asset manager namespace
        void Shutdown();

        /// create asset info
        bool CreateAssetInfo(const std::string &path);

#ifdef EDITOR
        /// AssetChange struct
        struct AssetChange
        {
            /// change
            FileSystem::Change mChange;

            /// flags
            FileSystem::Flags mFlags;

            /// last write time
            std::filesystem::file_time_type mLastWrite;

            /// name
            std::string mName;

            /// old name
            std::string mOldName;
        };

        /// process file changes
        void ProcessFileChanges(std::vector<FileSystem::FileChange> &&fileBatch);
#endif
    };

    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(AssetFactoryOld);

    // DEPRECATED, this is the old asset manager interface, we should move to the new one as soon as possible, but for now we keep it around for testing and reference

    /// AssetFactoryOld class
    class AssetFactoryOld
    {
        CLASS_NO_DEFAULT_CTOR(AssetFactoryOld);
        CLASS_NO_COPY_MOVE(AssetFactoryOld);

    public:
        /// constructs an assets factory implementation with priority
        explicit AssetFactoryOld(float priority) : mPriority(priority) {}

        /// gets priority
        [[nodiscard]] float Priority() const noexcept { return mPriority; }

        /// check if asset exists
        [[nodiscard]] virtual bool Exists(const std::filesystem::path &path) const = 0;

        /// import asset
        [[nodiscard]] virtual Expected<AssetPtr> Import(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const = 0;

    private:
        /// priority
        float mPriority;
    };

    /// AssetManagerOld namespace
    namespace AssetManagerOld
    {
        /// initialize asset manager namespace
        void Initialize(const EngineWeakPtr &engine);

        /// shutdown asset manager namespace
        void Shutdown();

        /// register an asset factory
        void RegisterFactory(const AssetFactoryOldPtr &assetFactory);

        /// import asset
        Expected<AssetPtr> Import(HashType type, const std::filesystem::path &path);
    };
}
