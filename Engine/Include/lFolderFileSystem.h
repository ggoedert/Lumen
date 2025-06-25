//==============================================================================================================================================================================
/// \file
/// \brief     FolderFileSystem interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lFileSystem.h"

/// Lumen namespace
namespace Lumen
{
    /// FolderFileSystem class
    class FolderFileSystem : public IFileSystem
    {
        CLASS_NO_DEFAULT_CTOR(FolderFileSystem);
        CLASS_NO_COPY_MOVE(FolderFileSystem);

    public:
        /// creates a smart pointer version of the folder file system
        static IFileSystemPtr MakePtr(const std::string &name, const std::string &path);

        /// opens a file on the specified path
        void Open(const std::string &path) const override;

    private:
        /// constructs a folder file system
        explicit FolderFileSystem(const std::string &name, const std::string &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
