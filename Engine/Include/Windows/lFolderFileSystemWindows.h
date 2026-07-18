//==============================================================================================================================================================================
/// \file
/// \brief     FolderFileSystemWindows interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lFolderFileSystem.h"

/// Lumen namespace
namespace Lumen
{
    /// FolderFileSystemWindows class
    class FolderFileSystemWindows : public FolderFileSystem
    {
        CLASS_NO_DEFAULT_CTOR(FolderFileSystemWindows);
        CLASS_NO_COPY_MOVE(FolderFileSystemWindows);

    public:
        /// constructs a folder file system windows
        explicit FolderFileSystemWindows(const std::filesystem::path &path);

        /// initialize file system
        void Initialize() override;

    private:
        /// get path
        const std::filesystem::path &Path() const override;

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
