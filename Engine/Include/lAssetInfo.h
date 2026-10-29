//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

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
        /// custom deleter type
        using Deleter = std::function<void(AssetInfo *)>;

        /// destructor
        ~AssetInfo();

        /// creates a smart pointer version of the AssetInfo (custom deletion)
        static AssetInfoPtr MakePtr(const std::filesystem::path &path, Deleter deleter);

        /// initialize assetinfo
        void Initialize();

        /// save assetinfo
        bool Save() const;

        /// load assetinfo
        bool Load();

        /// get path
        const std::filesystem::path &Path() const;

        /// get UUID
        Lumen::UUID UUID() const;

    private:
        /// constructs an assetinfo
        explicit AssetInfo(const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
