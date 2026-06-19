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
        /// creates a smart pointer version of the assetinfo
        static Expected<AssetInfoPtr> MakePtr(const std::filesystem::path &path);

        /// initialize assetinfo
        void Initialize();

        /// register assetinfo name / path
        static void Register(std::string_view name, std::string_view path);

        /// find assetinfo path from name
        static Expected<std::string_view> Find(std::string_view name);

        /// save assetinfo
        bool Save() const;

        /// load assetinfo
        bool Load();

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
