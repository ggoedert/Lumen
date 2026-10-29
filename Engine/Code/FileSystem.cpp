//==============================================================================================================================================================================
/// \file
/// \brief     file
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFileSystem.h"
#include "lStringMap.h"
#include "lConcurrentBatchQueue.h"
#include "lAssetManager.h"

using namespace Lumen;

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    /// file sytem state struct
    struct FileSytemState
    {
        CLASS_NO_COPY_MOVE(FileSytemState);

        /// default constructor
        explicit FileSytemState() = default;

        /// file id generator
        Id::Generator mFileIdGenerator;

        /// file systems
        StringMap<IFileSystemPtr> mFileSystems;

#ifdef EDITOR
        /// delayed file batch type, ordered by the time the change is allowed to be processed
        using DelayedFileBatch = std::multimap<steady_clock::time_point, FileSystem::FileChange>;

        /// per file name bookkeeping of the delayed batch, keeps the per change lookups O(1)
        struct DelayedName
        {
            /// pending Added / Modified change of this name (at most one, further ones are coalesced into it), std::nullopt if Renamed / Removed
            std::optional<DelayedFileBatch::iterator> mPending;

            /// latest process time of any pending change of this name, changes of one name are never reordered
            steady_clock::time_point mLastTime;

            /// number of pending changes of this name
            size_t mCount = 0;
        };

        /// file batch queue
        ConcurrentBatchQueue<FileSystem::FileChange> mFileBatchQueue;

        // delayed file batch
        DelayedFileBatch mDelayedFileBatch;

        /// delayed file batch index by name
        std::unordered_map<std::string, DelayedName> mDelayedNames;
#endif
    };

    /// global file state
    static std::unique_ptr<FileSytemState> gFileSytemState;

    /// checks if a path starts with a prefix
    bool StartsWith(const std::filesystem::path &path, const std::filesystem::path &prefix)
    {
        std::string pathStr = path.generic_string();
        std::string prefixStr = prefix.generic_string();
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

class HandleStreamBuf : public std::streambuf
{
public:
    /// adjust buffer size based on your performance needs
    explicit HandleStreamBuf(Id::Type handle, size_t bufferSize = 4096) : mHandler(handle), mBuffer(bufferSize)
    {
        // set the get area to be empty initially
        setg(mBuffer.data(), mBuffer.data(), mBuffer.data());
    }

protected:
    /// called when the stream needs more bytes
    int_type underflow() override
    {
        if (gptr() < egptr())
        {
            return traits_type::to_int_type(*gptr());
        }

        // read bytes
        size_t bytesRead = FileSystem::ReadBytes(mHandler, mBuffer.data(), mBuffer.size());

        if (bytesRead == 0)
        {
            return traits_type::eof();
        }

        // reset the buffer pointers
        setg(mBuffer.data(), mBuffer.data(), mBuffer.data() + bytesRead);
        return traits_type::to_int_type(*gptr());
    }

private:
    /// file handle
    Id::Type mHandler;

    /// internal buffer for reading
    std::vector<char> mBuffer;
};

/// initialize file namespace
void FileSystem::Initialize(const EngineWeakPtr &engine)
{
    L_ASSERT(!Hidden::gFileSytemState);
    Hidden::gFileSytemState = std::make_unique<Hidden::FileSytemState>();
}

/// shutdown file namespace
void FileSystem::Shutdown()
{
    L_ASSERT(Hidden::gFileSytemState);

    // the file systems are owned elsewhere and can outlive this state, stop them first so file watchers
    // can no longer report into a state that is about to be destroyed
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        fileSystem->Shutdown();
    }
    Hidden::gFileSytemState.reset();
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
    return filePath.lexically_normal();
}

/// register file system
void FileSystem::RegisterFileSystem(const std::filesystem::path &mountPoint, const IFileSystemPtr &fileSystem)
{
    L_ASSERT(Hidden::gFileSytemState);
    Hidden::gFileSytemState->mFileSystems.insert_or_assign(FileSystem::NormalizeDirPath(mountPoint).generic_string(), fileSystem);
}

#ifdef EDITOR
/// push file changes
void FileSystem::PushFileChangeBatch(std::vector<FileChange> &&fileBatch)
{
    L_ASSERT(Hidden::gFileSytemState);
    Hidden::gFileSytemState->mFileBatchQueue.PushBatch(std::move(fileBatch));
}

/// process file changes
void FileSystem::ProcessFileChanges()
{
    L_ASSERT(Hidden::gFileSytemState);

    steady_clock::time_point now = steady_clock::now();

    // pop file changes from the queue
    auto &state = *Hidden::gFileSytemState;
    std::list<std::vector<FileChange>> fileBatchQueue;
    if (state.mFileBatchQueue.PopBatchQueue(fileBatchQueue))
    {
        for (auto &incoming : fileBatchQueue)
        {
            for (auto &fileChange : incoming)
            {
                // ignore directory modified events
                if ((fileChange.mChange == Change::Modified) && (fileChange.mAttributes.mFlags.Has(Flag::Directory)))
                {
                    continue;
                }

                const bool addedOrModified = (fileChange.mChange == Change::Added) || (fileChange.mChange == Change::Modified);
                auto &nameState = state.mDelayedNames[fileChange.mName];

                // coalesce added / modified changes into the pending added / modified change of the same name
                if (addedOrModified && nameState.mPending.has_value())
                {
                    auto it = nameState.mPending.value();
                    it->second.mAttributes.mLastWrite = fileChange.mAttributes.mLastWrite;
                    it->second.mAttributes.mSize = fileChange.mAttributes.mSize;
                }
                else
                {
                    // otherwise insert it with a delay, never before an earlier pending change of the same name
                    steady_clock::time_point insertTime = now;
                    if (addedOrModified)
                    {
                        insertTime += milliseconds(100);
                    }
                    if (nameState.mCount > 0)
                    {
                        insertTime = std::max(nameState.mLastTime, insertTime);
                    }
                    auto inserted = state.mDelayedFileBatch.emplace(insertTime, std::move(fileChange));

                    nameState.mLastTime = insertTime;
                    ++nameState.mCount;

                    if (addedOrModified)
                    {
                        nameState.mPending = inserted;
                    }
                    else
                    {
                        // a removed or renamed event breaks the modification chain, we must clear the pending iterator so future additions don't coalesce backwards
                        nameState.mPending = std::nullopt;
                    }
                }
            }
        }
    }

    // process delayed file changes that are ready
    std::vector<FileChange> fileBatch;
    auto cutoff = state.mDelayedFileBatch.upper_bound(now);
    for (auto it = state.mDelayedFileBatch.begin(); it != cutoff; ++it)
    {
        auto nameIt = state.mDelayedNames.find(it->second.mName);
        if (nameIt != state.mDelayedNames.end())
        {
            if (nameIt->second.mPending.has_value() && (nameIt->second.mPending.value() == it))
            {
                nameIt->second.mPending = std::nullopt;
            }
            if (--nameIt->second.mCount == 0)
            {
                state.mDelayedNames.erase(nameIt);
            }
        }
        fileBatch.push_back(std::move(it->second));
    }
    state.mDelayedFileBatch.erase(state.mDelayedFileBatch.begin(), cutoff);

    // process file changes (process even if empty to allow for asset manager to process delayed changes)
    for (auto &fileChange : fileBatch)
    {
        switch (fileChange.mChange)
        {
        case Change::Added:
            Lumen::DebugLog::Info("fileChange: Added, {}", fileChange.mName);
            break;
        case Change::Modified:
            Lumen::DebugLog::Info("fileChange: Modified, {}", fileChange.mName);
            break;
        case Change::Renamed:
            Lumen::DebugLog::Info("fileChange: Renamed, {}, {}", fileChange.mOldName, fileChange.mName);
            break;
        case Change::Removed:
            Lumen::DebugLog::Info("fileChange: Removed, {}", fileChange.mName);
            break;
        }
    }
    AssetManager::ProcessFileChanges(std::move(fileBatch));
}
#endif

/// generates a new file id
Id::Type FileSystem::GenerateFileId()
{
    return Hidden::gFileSytemState->mFileIdGenerator.Next();
}

/// read serialized data from a path
bool FileSystem::ReadSerializedData(const std::filesystem::path &path, Serialized::Type &data)
{
    bool binary = FileSystem::IsPacked(path);
    Id::Type file = FileSystem::Open(path, false, binary);
    if (file == Id::Invalid)
    {
        Lumen::DebugLog::Error("Unable to open serialized file for reading, {}", path.generic_string());
        return false;
    }

    bool result = true;
    if (binary)
    {
        size_t fileSize = FileSystem::Size(file);
        std::vector<uint8_t> v(fileSize);
        if (FileSystem::ReadBytes(file, v.data(), fileSize) == fileSize)
        {
            try
            {
                data = Serialized::Type::from_cbor(v);
            }
            catch (const std::exception &e)
            {
                Lumen::DebugLog::Error("Unable to parse serialized file {}, error {}", path.generic_string(), e.what());
                result = false;
            }
        }
        else
        {
            Lumen::DebugLog::Error("Unable to read serialized file {}, short read", path.generic_string());
            result = false;
        }
    }
    else
    {
        try
        {
            data = Serialized::Type::parse(FileSystem::ReadText(file));
        }
        catch (const std::exception &e)
        {
            Lumen::DebugLog::Error("Unable to parse serialized file {}, error {}", path.generic_string(), e.what());
            result = false;
        }
    }
    FileSystem::Close(file);
    return result;
}

/// write serialized data to a path
bool FileSystem::WriteSerializedData(const std::filesystem::path &path, const Serialized::Type &data)
{
    bool binary = FileSystem::IsPacked(path);
    Id::Type file = FileSystem::Open(path, true, binary);
    if (file == Id::Invalid)
    {
        Lumen::DebugLog::Error("Unable to open serialized file for writing, {}", path.generic_string());
        return false;
    }

    bool result = true;
    if (binary)
    {
        std::vector<uint8_t> serData = Serialized::Type::to_cbor(data);
        result = FileSystem::WriteBytes(file, serData.data(), serData.size());
    }
    else
    {
        result = FileSystem::WriteText(file, data.dump(4));
    }

    FileSystem::Close(file);
    if (!result)
    {
        Lumen::DebugLog::Error("Unable to write serialized file {}", path.generic_string());
    }
    return result;
}

/// read asset info from a path
bool FileSystem::ReadAssetInfo(const std::filesystem::path &path, UUID &uuid, Serialized::Type *data)
{
    bool binary = FileSystem::IsPacked(path);
    Id::Type file = FileSystem::Open(path, false, binary);
    if (file == Id::Invalid)
    {
        Lumen::DebugLog::Error("Unable to open asset info file for reading, {}", path.generic_string());
        return false;
    }

    bool result = true;
    if (binary)
    {
        size_t fileSize = FileSystem::Size(file);
        if (fileSize < sizeof(UUID))
        {
            Lumen::DebugLog::Error("Asset info file {} is too small to contain a UUID", path.generic_string());
            FileSystem::Close(file);
            return false;
        }

        uuid = 0;
        std::vector<uint8_t> u(sizeof(UUID));
        if (FileSystem::ReadBytes(file, u.data(), sizeof(UUID)) == sizeof(UUID))
        {
            for (std::size_t i = 0; i < sizeof(UUID); ++i)
            {
                uuid = static_cast<UUID>((uuid << 8) | static_cast<UUID>(u[i]));
            }
        }
        else
        {
            FileSystem::Close(file);
            return false;
        }

        if (data)
        {
            size_t dataSize = fileSize - sizeof(UUID);
            if (dataSize > 0)
            {
                std::vector<uint8_t> v(dataSize);
                if (FileSystem::ReadBytes(file, v.data(), dataSize) == dataSize)
                {
                    try
                    {
                        *data = Serialized::Type::from_cbor(v);
                    }
                    catch (const std::exception &e)
                    {
                        Lumen::DebugLog::Error("Unable to parse asset info file {}, error {}", path.generic_string(), e.what());
                        result = false;
                    }
                }
            }
        }
    }
    else
    {
        // first line is: "<uuid>": <start of the json data>
        std::string uuidText = FileSystem::ReadText(file, 1);
        if (uuidText.length() < 16)
        {
            Lumen::DebugLog::Error("Asset info file {} is damaged", path.generic_string());
            FileSystem::Close(file);
            return false;
        }
        uuid = UUIDFromString(uuidText.substr(1, uuidText.length() - 6));
        uuidText = uuidText.substr(16);

        if (data)
        {
            try
            {
                *data = Serialized::Type::parse(uuidText + FileSystem::ReadText(file));
            }
            catch (const std::exception &e)
            {
                Lumen::DebugLog::Error("Unable to parse asset info file {}, error {}", path.generic_string(), e.what());
                result = false;
            }
        }
    }
    FileSystem::Close(file);
    return result;
}

/// write asset info to a path
bool FileSystem::WriteAssetInfo(const std::filesystem::path &path, UUID uuid, const Serialized::Type &data)
{
    bool binary = FileSystem::IsPacked(path);
    Id::Type file = FileSystem::Open(path, true, binary);
    if (file == Id::Invalid)
    {
        Lumen::DebugLog::Error("Unable to open serialized file for writing, {}", path.generic_string());
        return false;
    }

    bool result = true;
    if (binary)
    {
        std::vector<uint8_t> serUUID(sizeof(UUID));
        for (std::size_t i = 0; i < sizeof(UUID); ++i)
        {
            serUUID[i] = static_cast<uint8_t>((uuid >> ((sizeof(UUID) - 1 - i) * 8)) & 0xff);
        }
        std::vector<uint8_t> serData = Serialized::Type::to_cbor(data);
        result = FileSystem::WriteBytes(file, serUUID.data(), serUUID.size()) && FileSystem::WriteBytes(file, serData.data(), serData.size());
    }
    else
    {
        result = FileSystem::WriteText(file, "\"" + UUIDToString(uuid) + "\": ") && FileSystem::WriteText(file, data.dump(4));
    }

    FileSystem::Close(file);
    return result;
}

/// checks if a path is packed
bool FileSystem::IsPacked(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            return fileSystem->Packed();
        }
    }
    Lumen::DebugLog::Error("No registered file system for path {}", path.generic_string());
    return false;
}

/// check if a file exists
bool FileSystem::Exists(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->Exists(relativePath);
        }
    }
    return false;
}

/// rename a file
bool FileSystem::Rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(oldPath, mountPoint))
        {
            std::filesystem::path relativeOldPath = oldPath.lexically_relative(mountPoint);
            std::filesystem::path relativeNewPath = newPath.lexically_relative(mountPoint);
            return fileSystem->Rename(relativeOldPath.generic_string(), relativeNewPath.generic_string());
        }
    }
    Lumen::DebugLog::Error("No registered file system for path {}", oldPath.generic_string());
    return false;
}

/// delete a file or an empty directory
bool FileSystem::Delete(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->Delete(relativePath);
        }
    }
    Lumen::DebugLog::Error("No registered file system for path {}", path.generic_string());
    return false;
}

/// get file attributes
FileSystem::FileAttributes FileSystem::GetAttributes(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->GetAttributes(relativePath);
        }
    }
    return FileAttributes();
}

/// ensure a directory exists
bool FileSystem::EnsureDirectory(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->EnsureDirectory(relativePath);
        }
    }
    return false;
}

/// list files in a directory
std::vector<FileSystem::FileEntry> FileSystem::ListFiles(const std::filesystem::path &path)
{
    L_ASSERT(Hidden::gFileSytemState);
    std::vector<FileEntry> files;
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            std::vector<FileEntry> mountFiles = fileSystem->ListFiles(relativePath);
            files.insert(files.end(), mountFiles.begin(), mountFiles.end());
        }
    }
    return files;
}

/// opens a file on the specified path
Id::Type FileSystem::Open(const std::filesystem::path &path, bool write, bool binary)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (Hidden::StartsWith(path, mountPoint))
        {
            std::filesystem::path relativePath = path.lexically_relative(mountPoint);
            return fileSystem->Open(relativePath, write, binary);
        }
    }
    Lumen::DebugLog::Error("No registered file system for path {}", path.generic_string());
    return Id::Invalid;
}

/// closes a file handle
void FileSystem::Close(const Id::Type handle)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
        {
            fileSystem->Close(handle);
            return;
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
}

/// reads bytes from a file handle
size_t FileSystem::ReadBytes(const Id::Type handle, void *buffer, const size_t size)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
        {
            return fileSystem->ReadBytes(handle, buffer, size);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return 0;
}

/// writes bytes to a file handle
bool FileSystem::WriteBytes(const Id::Type handle, const void *buffer, const size_t size)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
        {
            return fileSystem->WriteBytes(handle, buffer, size);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return false;
}

/// reads text from a file handle
std::string FileSystem::ReadText(const Id::Type handle, int lineCount)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
        {
            return fileSystem->ReadText(handle, lineCount);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return {};
}

/// writes text to a file handle
bool FileSystem::WriteText(const Id::Type handle, const std::string &text)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
        {
            return fileSystem->WriteText(handle, text);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return false;
}

/// gets the current position in the file by handle
size_t FileSystem::Tell(const Id::Type handle)
{
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
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
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
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
    L_ASSERT(Hidden::gFileSytemState);
    for (auto &[mountPoint, fileSystem] : Hidden::gFileSytemState->mFileSystems)
    {
        if (fileSystem->Handles(handle))
        {
            return fileSystem->Size(handle);
        }
    }
    Lumen::DebugLog::Error("No registered file system for file handle {}", handle);
    return -1;
}
