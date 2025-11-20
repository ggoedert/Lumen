//==============================================================================================================================================================================
/// \file
/// \brief     asset manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lExpected.h"
#include "lAsset.h"
#include "lEngine.h"

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

        /// import the asset
        [[nodiscard]] virtual Lumen::Expected<AssetPtr> Import(EngineWeakPtr &engine) = 0;
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
        /// AssetChange struct
        struct AssetChange
        {
            /// type
            enum class Type { Added, Modified, Renamed, Removed };

            /// change
            Type change;

            /// name
            std::string name;

            /// old name
            std::string oldName;
        };

        /// initialize asset manager namespace
        void Initialize(const EngineWeakPtr &engine);

        /// shutdown asset manager namespace
        void Shutdown();

        /// register an asset factory
        void RegisterFactory(const AssetFactoryPtr &assetFactory);

        /// process asset changes
        void ProcessAssetChanges(std::list<std::vector<AssetChange>> &&batchQueue);

        /// import asset
        Expected<AssetPtr> Import(HashType type, const std::filesystem::path &path);
    };
}
