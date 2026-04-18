//==============================================================================================================================================================================
/// \file
/// \brief     file interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lId.h"
#include "lFlags.h"
#include "lSerializedData.h"

#include <filesystem>

/// Lumen namespace
namespace Lumen
{
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(IFileSystem);

    /// FileSystem namespace
    namespace FileSystem
    {
        /// change type
        enum class Change { Added, Modified, Renamed, Removed };

        /// change flag type
        enum class Flag : byte { None = 0x0, File = 0x1, Directory = 0x2 };

        /// change flags type
        using Flags = Lumen::Flags<Flag>;

        /// FileChange struct
        struct FileChange
        {
            /// change
            Change mChange;

            /// flags
            Flags mFlags;

            /// name
            std::string mName;

            /// old name
            std::string mOldName;
        };

#ifdef EDITOR
        /// AssetChange struct
        struct AssetChange
        {
            /// change
            Change mChange;

            /// flags
            Flags mFlags;

            /// name
            std::string mName;

            /// old name
            std::string mOldName;
        };
#endif

        /// initialize file namespace
        void Initialize(const EngineWeakPtr &engine);

        /// shutdown file namespace
        void Shutdown();

        /// normalize a directory path
        const std::filesystem::path NormalizeDirPath(const std::filesystem::path &dirPath);

        /// normalize a file path
        const std::filesystem::path NormalizeFilePath(const std::filesystem::path &filePath);

        /// register a file system
        void RegisterFileSystem(const std::filesystem::path &mountPoint, const IFileSystemPtr &fileSystem);

        /// push file changes
        void PushFileChangeBatch(std::vector<FileChange> &&fileBatch);

        /// process file changes
        void ProcessFileChanges();

        /// generates a new file id
        Id::Type GenerateFileId();

        /// read serialized data from a path
        bool ReadSerializedData(const std::filesystem::path &path, Serialized::Type &data);

        /// write serialized data to a path
        bool WriteSerializedData(const std::filesystem::path &path, const Serialized::Type &data);

        /// checks if a path is packed
        bool IsPacked(const std::filesystem::path &path);

        /// check if a file exists
        bool Exists(const std::filesystem::path &path);

        /// opens a file on the specified path
        Id::Type Open(const std::filesystem::path &path, bool binary);

        /// closes a file handle
        void Close(const Id::Type handle);

        /// reads bytes from a file handle
        size_t ReadBytes(const Id::Type handle, void *buffer, const size_t size = SIZE_MAX);

        /// writes bytes to a file handle
        bool WriteBytes(const Id::Type handle, const void *buffer, const size_t size);

        /// reads text from a file handle
        std::string ReadText(const Id::Type handle, int lineCount = -1);

        /// writes text to a file handle
        bool WriteText(const Id::Type handle, const std::string &text);

        /// reads metafile record from a file handle
        bool ReadMetafileRecord(const Id::Type handle, Serialized::Type &record);

        /// writes metafile record to a file handle
        bool WriteMetafileRecord(const Id::Type handle, const Serialized::Type &record);

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
        virtual Id::Type Open(const std::filesystem::path &path, bool binary) = 0;

        /// closes a file handle
        virtual void Close(const Id::Type handle) = 0;

        /// reads bytes from a file handle
        virtual size_t ReadBytes(const Id::Type handle, void *buffer, const size_t size = SIZE_MAX) = 0;

        /// writes bytes to a file handle
        virtual bool WriteBytes(const Id::Type handle, const void *buffer, const size_t size) = 0;

        /// reads text from a file handle
        virtual std::string ReadText(const Id::Type handle, int lineCount = -1) = 0;

        /// writes text to a file handle
        virtual bool WriteText(const Id::Type handle, const std::string &text) = 0;

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
