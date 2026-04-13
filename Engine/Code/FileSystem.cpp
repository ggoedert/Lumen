//==============================================================================================================================================================================
/// \file
/// \brief     file
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFileSystem.h"
#include "lStringMap.h"
#include "lConcurrentBatchQueue.h"
#include "lEngine.h"

using namespace Lumen;

constexpr std::chrono::milliseconds MetaDelay { 500 };

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    struct FileState
    {
        CLASS_NO_COPY_MOVE(FileState);

        /// default constructor
        explicit FileState() = default;

        /// engine pointer
        EngineWeakPtr mEngine;

        /// file id generator
        Id::Generator mFileIdGenerator;

        /// file change mapFileChange
        std::multimap<std::chrono::system_clock::time_point, FileSystem::AssetChange> mAssetChangeMap;

        /// file systems
        StringMap<IFileSystemPtr> mFileSystems;
    };

    static std::unique_ptr<FileState> gFileState;

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

#ifdef EDITOR
    /// file batch queue
    ConcurrentBatchQueue<FileSystem::FileChange> gFileBatchQueue;
#endif
}

/// initialize file namespace
void FileSystem::Initialize(const EngineWeakPtr &engine)
{
    L_ASSERT(!Hidden::gFileState);
    Hidden::gFileState = std::make_unique<Hidden::FileState>();
    Hidden::gFileState->mEngine = engine;
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

/// push file changes
void FileSystem::PushFileChangeBatch(std::vector<FileChange> &&fileBatch)
{
    Hidden::gFileBatchQueue.PushBatch(std::move(fileBatch));
}

/// process file changes
void FileSystem::ProcessFileChanges()
{
    using clock = std::chrono::system_clock;
    using time_point = clock::time_point;
    using ms = std::chrono::milliseconds;

    auto now = clock::now();
    auto &map = Hidden::gFileState->mAssetChangeMap;
    std::vector<AssetChange> assetBatch;

    std::list<std::vector<FileChange>> fileBatchQueue;
    bool dotest = false;
    if (Hidden::gFileBatchQueue.PopBatchQueue(fileBatchQueue))
    {
        for (auto &batch : fileBatchQueue)
        {
            for (auto &fileChange : batch)
            {
                // determine meta and base file existence
                auto ext = std::filesystem::path(fileChange.mName).extension().string();
                bool isMeta = (ext.size() == 5) &&
                    std::equal(ext.begin(), ext.end(), ".meta", [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
                bool hasMeta = isMeta;
                bool hasBase = !isMeta;
                if (isMeta)
                {
                    hasBase = std::filesystem::exists(std::filesystem::path(fileChange.mName.substr(0, fileChange.mName.size() - 5)));
                }
                else
                {
                    hasMeta = std::filesystem::exists(std::filesystem::path(fileChange.mName + ".meta"));
                }

                if (!isMeta)
                {
                    if (hasBase)
                    {
                        AssetChange assetChange;
                        switch (fileChange.mChange)
                        {
                        case FileSystem::Change::Added:
                            assetChange = { FileSystem::Change::Added, fileChange.mFlags, fileChange.mName, "" };
                            if (hasMeta)
                            {
                                Lumen::DebugLog::Detail("Added file with meta file, {}, metafile exists", assetChange.mName);
                                assetBatch.push_back(assetChange);
                            }
                            else
                            {
                                dotest = true;
                                map.emplace(clock::now() + MetaDelay, assetChange);
                            }
                            break;
                        case FileSystem::Change::Modified:
                            assetBatch.push_back({ FileSystem::Change::Modified, fileChange.mFlags, fileChange.mName, "" });
                            break;
                        case FileSystem::Change::Renamed:
                            assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mFlags, fileChange.mName, fileChange.mOldName });
                            break;
                        case FileSystem::Change::Removed:
                            assetBatch.push_back({ FileSystem::Change::Removed, fileChange.mFlags, fileChange.mName, "" });
                            break;
                        }
                    }
                }
            }
        }
    }

    if (dotest)
    {
        // CREATE THE Assets/serializer_test.txt
    }

    for (auto it = map.begin(); it != map.end(); )
    {
        auto &[processTime, assetChange] = *it;

        if (std::filesystem::exists(std::filesystem::path(assetChange.mName + ".meta")))
        {
            Lumen::DebugLog::Detail("Added file with meta file, {}, metafile late arrival", assetChange.mName);
            assetBatch.push_back(assetChange);
            it = map.erase(it);
        }
        else if (processTime <= now)
        {
            // CREATE THE METAFILE
            Lumen::DebugLog::Detail("Added file without meta file, {}, created metafile", assetChange.mName);
            assetBatch.push_back(assetChange);
            it = map.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (!assetBatch.empty())
    {
        if (auto engineLock = Hidden::gFileState->mEngine.lock())
        {
            engineLock->ProcessAssetChanges(std::move(assetBatch));
        }
    }
}

/// generates a new file id
Id::Type FileSystem::GenerateFileId()
{
    return Hidden::gFileState->mFileIdGenerator.Next();
}

/// read serialized data from a path
bool FileSystem::ReadSerializedData(const std::filesystem::path &path, Serialized::Type &data, bool &packed)
{
    packed = FileSystem::IsPacked(path);
    if (packed)
    {
        Lumen::DebugLog::Error("Packed serialized data loading not implemented yet, {}", path.string());
    }
    else
    {
        Id::Type file = FileSystem::Open(path);
        if (file != Id::Invalid)
        {
            data = Serialized::Type::parse(FileSystem::ReadLines(file));
            FileSystem::Close(file);
            return true;
        }
        else
        {
            Lumen::DebugLog::Error("Unable to open scene file for reading, {}", path.string());
        }
    }
    return false;
}

/// write serialized data to a path
bool FileSystem::WriteSerializedData(const std::filesystem::path &path, const Serialized::Type &data, bool packed)
{
    if (FileSystem::IsPacked(path) != packed)
    {
        Lumen::DebugLog::Error("Packed flag does not match file system type for path {}, packed flag: {}, file system packed: {}", path.string(), packed, !packed);
        return false;
    }

    if (packed)
    {
        Lumen::DebugLog::Error("Packed serialized data saving not implemented yet, {}", path.string());
    }
    else
    {
        Id::Type file = FileSystem::Open(path);
        if (file != Id::Invalid)
        {
            FileSystem::WriteLines(file, data.dump(4));
            FileSystem::Close(file);
            return true;
        }
        else
        {
            Lumen::DebugLog::Error("Unable to open scene file for writing, {}", path.string());
        }
    }
    return false;
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
Id::Type FileSystem::Open(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->Open(relativePath.string());
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

/// reads lines from a file handle
std::string FileSystem::ReadLines(const Id::Type handle, int lineCount)
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

/// writes lines to a file handle
bool FileSystem::WriteLines(const Id::Type handle, const std::string &lines)
{
    L_ASSERT(Hidden::gFileState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileState->mFileSystems)
    {
        if (fileSystem->HandlesFileId(handle))
        {
            return fileSystem->WriteLines(handle, lines);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return false;
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
