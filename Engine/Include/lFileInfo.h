//==============================================================================================================================================================================
/// \file
/// \brief     FileInfo interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lAssetInfo.h"
#include "lExpected.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(FileInfo);
    CLASS_WEAK_PTR_DEF(FileInfo);

    /// FileInfo class, the asset info of an asset that comes from a file system (kept in the .info file next to its source), or that a game build
    /// recreates from the asset info stored in a Library entry
    class FileInfo : public AssetInfo
    {
        CLASS_NO_DEFAULT_CTOR(FileInfo);
        CLASS_NO_COPY_MOVE(FileInfo);

    public:
        /// custom deleter type
        using Deleter = std::function<void(FileInfo *)>;

        /// destructor
        ~FileInfo() override;

        /// creates a smart pointer version of the FileInfo, its UUID comes from Initialize or Load (custom deletion)
        static FileInfoPtr MakePtr(const std::filesystem::path &path, HashType type, Deleter deleter);

        /// creates a smart pointer version of the FileInfo from a serialized asset info, the one stored in a Library entry (custom deletion)
        static Expected<FileInfoPtr> Deserialize(const Serialized::Type &in, bool packed, Deleter deleter);

        /// initialize fileinfo, gives it a new UUID
        void Initialize();

        /// save fileinfo
        bool Save() const;

        /// load fileinfo
        bool Load();

        /// get path
        [[nodiscard]] const std::filesystem::path &Path() const override;

        /// get UUID
        [[nodiscard]] Lumen::UUID UUID() const override;

        /// get the asset type
        [[nodiscard]] HashType Type() const override;

    private:
        /// constructs a fileinfo
        explicit FileInfo(const std::filesystem::path &path, HashType type, Lumen::UUID uuid);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
