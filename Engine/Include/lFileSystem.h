//==============================================================================================================================================================================
/// \file
/// \brief     file interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lId.h"

#include <filesystem>

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(IFileSystem);

    /// FileSystem namespace
    namespace FileSystem
    {
        /// file mode
        enum class FileMode { Text, Binary };

        /// initialize file namespace
        void Initialize();

        /// shutdown file namespace
        void Shutdown();

        /// normalize a directory path
        const std::filesystem::path NormalizeDirPath(const std::filesystem::path &dirPath);

        /// normalize a file path
        const std::filesystem::path NormalizeFilePath(const std::filesystem::path &filePath);

        /// register a file system
        void RegisterFileSystem(const std::filesystem::path &mountPoint, const IFileSystemPtr &fileSystem);

        /// generates a new file id
        Id::Type GenerateFileId();

        /// reads serialized data from a path
        std::pair<std::string, bool> ReadSerializedData(const std::filesystem::path &path);

        /// checks if a path is packed
        bool IsPacked(const std::filesystem::path &path);

        /// check if a file exists
        bool Exists(const std::filesystem::path &path);

        /// opens a file on the specified path
        Id::Type Open(const std::filesystem::path &path, const FileMode mode);

        /// closes a file handle
        void Close(const Id::Type handle);

        /// reads bytes from a file handle
        size_t ReadBytes(const Id::Type handle, const void *buffer, const size_t size);

        /// reads a line from a file handle
        std::vector<std::string> ReadLines(const Id::Type handle, int lineCount = -1);

        /// gets the current position in the file by handle
        size_t Tell(const Id::Type handle);

        /// seeks to a position in the file by handle
        void Seek(const Id::Type handle, const size_t position);

        /// gets the size of the file by handle
        size_t Size(const Id::Type handle);
    };

    /// FileSystem interface class
    class IFileSystem
    {
        CLASS_NO_COPY_MOVE(IFileSystem);

    public:
        /// whether this file system is packed
        virtual bool Packed() const = 0;

        /// whether this file system handles the specified file handle
        virtual bool HandlesFileId(Id::Type handle) = 0;

        /// check if a file exists
        virtual bool Exists(const std::filesystem::path &path) = 0;

        /// opens a file on the specified path
        virtual Id::Type Open(const std::filesystem::path &path, const FileSystem::FileMode mode) = 0;

        /// closes a file handle
        virtual void Close(const Id::Type handle) = 0;

        /// reads bytes from a file handle
        virtual size_t ReadBytes(const Id::Type handle, const void *buffer, const size_t size) = 0;

        /// reads a line from a file handle
        virtual std::vector<std::string> ReadLines(const Id::Type handle, int lineCount = -1) = 0;

        /// gets the current position in the file by handle
        virtual size_t Tell(const Id::Type handle) = 0;

        /// seeks to a position in the file by handle
        virtual void Seek(const Id::Type handle, const size_t position) = 0;

        /// gets the size of the file by handle
        virtual size_t Size(const Id::Type handle) = 0;

    protected:
        /// default constructor
        explicit IFileSystem() = default;

        /// pure virtual destructor
        virtual ~IFileSystem() = default;
    };
}
