//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lUUID.h"
#include "lHash.h"

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

        /// creates a smart pointer version of the AssetInfo of a builtin asset, defined in code, it has no info file (custom deletion)
        static AssetInfoPtr MakeBuiltinPtr(const std::filesystem::path &path, HashType type, Lumen::UUID uuid, Deleter deleter);

        /// whether a path names a builtin asset (procedural, no source in a file system), builtin paths start with '|'
        [[nodiscard]] static bool IsBuiltinPath(const std::filesystem::path &path);

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

        /// whether the assetinfo is the one of a builtin asset
        [[nodiscard]] bool Builtin() const;

        /// get the asset type, only known for builtin assets (a source asset gets its type from its extension)
        [[nodiscard]] HashType Type() const;

    private:
        /// constructs an assetinfo
        explicit AssetInfo(const std::filesystem::path &path);

        /// constructs a builtin assetinfo
        explicit AssetInfo(const std::filesystem::path &path, HashType type, Lumen::UUID uuid);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
