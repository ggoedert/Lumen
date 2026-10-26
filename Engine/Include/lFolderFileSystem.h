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

        /// initialize file system
        void Initialize(bool packed) override;

        /// whether this file system is packed
        bool Packed() const override;

        /// whether this file system handles the specified file handle
        bool Handles(Id::Type handle) override;

        /// check if a file exists
        bool Exists(const std::filesystem::path &path) override;

        /// rename a file
        bool Rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath) override;

        /// get file attributes
        FileSystem::FileAttributes GetAttributes(const std::filesystem::path &path) override;

        /// ensure a directory exists
        bool EnsureDirectory(const std::filesystem::path &path) override;

        /// list files in a directory
        std::vector<FileSystem::FileEntry> ListFiles(const std::filesystem::path &path) override;

        /// opens a file on the specified path
        Id::Type Open(const std::filesystem::path &path, bool write, bool binary) override;

        /// closes a file handle
        void Close(const Id::Type handle) override;

        /// reads bytes from a file handle
        size_t ReadBytes(const Id::Type handle, void *buffer, const size_t size = SIZE_MAX) override;

        /// writes bytes to a file handle
        bool WriteBytes(const Id::Type handle, const void *buffer, const size_t size) override;

        /// reads text from a file handle
        std::string ReadText(const Id::Type handle, const int lineCount = -1) override;

        /// writes text to a file handle
        bool WriteText(const Id::Type handle, const std::string &text) override;

        /// gets the current position in the file by handle
        size_t Tell(const Id::Type handle) override;

        /// seeks to a position in the file by handle
        void Seek(const Id::Type handle, const size_t position) override;

        /// gets the size of the file by handle
        size_t Size(const Id::Type handle) override;

    protected:
        /// constructs a folder file system
        explicit FolderFileSystem(const std::filesystem::path &path);

        /// destroys folder file system
        ~FolderFileSystem() override;

    private:
        /// get path
        virtual const std::filesystem::path &Path() const = 0;

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
