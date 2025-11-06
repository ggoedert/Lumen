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
        static IFileSystemPtr MakePtr(const std::filesystem::path &path);

        /// whether this file system is packed
        bool Packed() const override;

        /// whether this file system handles the specified file handle
        bool HandlesFileId(Id::Type handle) override;

        /// opens a file on the specified path
        Id::Type Open(const std::filesystem::path &path, const FileSystem::FileMode mode) override;

        /// closes a file handle
        void Close(const Id::Type handle) override;

        /// reads bytes from a file handle
        size_t ReadBytes(const Id::Type handle, const void *buffer, const size_t size) override;

        /// reads a line from a file handle
        std::vector<std::string> ReadLines(const Id::Type handle, const int lines = -1) override;

        /// gets the current position in the file by handle
        size_t Tell(const Id::Type handle) override;

        /// seeks to a position in the file by handle
        void Seek(const Id::Type handle, const size_t position) override;

        /// gets the size of the file by handle
        size_t Size(const Id::Type handle) override;

    private:
        /// constructs a folder file system
        explicit FolderFileSystem(const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
