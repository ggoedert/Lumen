//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lExpected.h"
#include "lUUID.h"

/// \cond
#include <filesystem>
/// \endcond

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(AssetInfo);
    CLASS_WEAK_PTR_DEF(AssetInfo);

    /// AssetInfo class
    class AssetInfo
    {
        CLASS_NO_DEFAULT_CTOR(AssetInfo);
        CLASS_NO_COPY_MOVE(AssetInfo);

    public:
        /// get or create a smart pointer version of the assetinfo from smart pointer cache
        static Expected<AssetInfoPtr> AcquirePtr(const std::filesystem::path &path);

        /// register asset info
        static void Register(const std::filesystem::path &path);

        /// rename asset info
        static void Rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);

        /// remove asset info
        static void Remove(const std::filesystem::path &path);

        /// get path
        const std::filesystem::path &Path() const;

        /// get UUID
        const UUID UUID() const;

    private:
        /// constructs an assetinfo
        explicit AssetInfo(const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
