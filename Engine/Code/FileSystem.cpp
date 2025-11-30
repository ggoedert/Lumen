//==============================================================================================================================================================================
/// \file
/// \brief     file
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFileSystem.h"
#include "lStringMap.h"

using namespace Lumen;

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    struct FileState
    {
        CLASS_NO_COPY_MOVE(FileState);

        /// default constructor
        explicit FileState() = default;

        /// file systems
        StringMap<IFileSystemPtr> mFileSystems;
    };

    static std::unique_ptr<FileState> gFileState;

    static Id::Generator gFileIdGenerator;

    /// checks if a path starts with a prefix
    bool StartsWith(const std::filesystem::path &path, const std::filesystem::path &prefix)
    {
        std::string pathStr = path.string();
        std::string prefixStr = prefix.string();
        for (auto itPath = pathStr.begin(), itPrefix = prefixStr.begin(); itPrefix != prefixStr.end(); ++itPath, ++itPrefix)
        {
            if (itPath == pathStr.end() || *itPath != *itPrefix)
            {
                return false;
            }
        }
        return true;
    }
}

/// initialize file namespace
void FileSystem::Initialize()
{
    L_ASSERT(!Hidden::gFileState);
    Hidden::gFileState = std::make_unique<Hidden::FileState>();
}

/// shutdown file namespace
void FileSystem::Shutdown()
{
    L_ASSERT(Hidden::gFileState);
    Hidden::gFileState.reset();
}

/// normalize a directory path
const std::filesystem::path FileSystem::NormalizeDirPath(const std::filesystem::path &dirPath)
{
    // trivial case
    if (dirPath.empty())
        return "./";

    // normalize and use forward slashes regardless of platform
    std::string result = dirPath.lexically_normal().generic_string();

    // ensure trailing slash
    if (result.back() != '/')
        result += '/';

    return result;
}

/// normalize a file path
const std::filesystem::path FileSystem::NormalizeFilePath(const std::filesystem::path &filePath)
{
    // trivial case
    return filePath.lexically_normal().generic_string();
}

/// register file system
void FileSystem::RegisterFileSystem(const std::filesystem::path &mountPoint, const IFileSystemPtr &fileSystem)
{
    L_ASSERT(Hidden::gFileState);
    Hidden::gFileState->mFileSystems.insert_or_assign(FileSystem::NormalizeDirPath(mountPoint).string(), fileSystem);
}

/// generates a new file id
Id::Type FileSystem::GenerateFileId()
{
    return Hidden::gFileIdGenerator.Next();
}

/// reads serialized data from a path
std::pair<std::string, bool> FileSystem::ReadSerializedData(const std::filesystem::path &path)
{
    std::string serializedData;
    bool packed = FileSystem::IsPacked(path);
    if (packed)
    {
        Lumen::DebugLog::Error("Packed serialized data loading not implemented yet, {}", path.string());
    }
    else
    {
        Id::Type file = FileSystem::Open(path, FileSystem::FileMode::Text);
        if (file != Id::Invalid)
        {
            std::vector<std::string> lines = FileSystem::ReadLines(file);
            for (const auto &line : lines)
            {
                serializedData += line;
            }
            FileSystem::Close(file);
        }
        else
        {
            Lumen::DebugLog::Error("Unable to open scene file for reading, {}", path.string());
        }
    }
    //packed = true; //@REVIEW@ testing!
    return { serializedData, packed };
}

/// checks if a path is packed
bool FileSystem::IsPacked(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            return fileSystem->Packed();
        }
    }
    Lumen::DebugLog::Error("No registered file system for path {}", path.string());
    return false;
}

/// check if a file exists
bool FileSystem::Exists(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->Exists(relativePath.string());
        }
    }
    return false;
}

/// opens a file on the specified path
Id::Type FileSystem::Open(const std::filesystem::path &path, const FileSystem::FileMode mode)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->Open(relativePath.string(), mode);
        }
    }
    Lumen::DebugLog::Error("No registered file system for path {}", path.string());
    return Id::Invalid;
}

/// closes a file handle
void FileSystem::Close(const Id::Type handle)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (fileSystem->HandlesFileId(handle))
        {
            fileSystem->Close(handle);
            return;
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
}

/// reads bytes from a file handle
size_t FileSystem::ReadBytes(const Id::Type handle, const void *buffer, const size_t size)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (fileSystem->HandlesFileId(handle))
        {
            return fileSystem->ReadBytes(handle, buffer, size);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return 0;
}

/// reads a line from a file handle
std::vector<std::string> FileSystem::ReadLines(const Id::Type handle, int lineCount)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (fileSystem->HandlesFileId(handle))
        {
            return fileSystem->ReadLines(handle, lineCount);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return {};
}

/// gets the current position in the file by handle
size_t FileSystem::Tell(const Id::Type handle)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (fileSystem->HandlesFileId(handle))
        {
            return fileSystem->Tell(handle);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return -1;
}

/// seeks to a position in the file by handle
void FileSystem::Seek(const Id::Type handle, const size_t position)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (fileSystem->HandlesFileId(handle))
        {
            fileSystem->Seek(handle, position);
            return;
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
}

/// gets the size of the file by handle
size_t FileSystem::Size(const Id::Type handle)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (fileSystem->HandlesFileId(handle))
        {
            return fileSystem->Size(handle);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return -1;
}
