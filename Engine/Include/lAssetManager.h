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

        /// import asset
        [[nodiscard]] virtual Expected<AssetPtr> Import(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const = 0;

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

        /// import asset
        Expected<AssetPtr> Import(HashType type, const std::filesystem::path &path);
    };
}
