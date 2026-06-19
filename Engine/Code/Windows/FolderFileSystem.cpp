//==============================================================================================================================================================================
/// \file
/// \brief     FolderFileSystem
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFolderFileSystem.h"

/// \cond
#include <fstream>
#include <unordered_map>
/// \endcond

using namespace Lumen;

/// FolderFileSystem::Impl class
class FolderFileSystem::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class FolderFileSystem;

public:
    /// constructs a folder file system implementation
    explicit Impl(const std::filesystem::path &path) : mPath(path) {}

    /// initialize file system
    void Initialize()
    {
        std::vector<FileSystem::FileChange> fileBatch;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(mPath))
        {
            if (entry.is_regular_file() || entry.is_directory())
            {
                FileSystem::Flags flags = entry.is_directory() ? FileSystem::Flag::Directory : FileSystem::Flag::File;
                std::string filename = entry.path().lexically_relative(mPath).generic_string();
                fileBatch.push_back({ FileSystem::Change::Added, flags, (mPath / filename).string(), "" });
            }
        }
        if (!fileBatch.empty())
        {
            FileSystem::PushFileChangeBatch(std::move(fileBatch));
        }
    }

    /// whether this file system is packed
    bool Packed() const
    {
        return false;
    }

    /// whether this file system handles the specified file handle
    bool Handles(Id::Type handle)
    {
        return mOpenFiles.find(handle) != mOpenFiles.end();
    }

    /// check if a file exists
    bool Exists(const std::filesystem::path &path)
    {
        std::filesystem::path fullPath = mPath / path;
        return std::filesystem::exists(fullPath);
    }

    /// list files in a directory
    std::vector<FileSystem::FileEntry> ListFiles(const std::filesystem::path &path)
    {
        std::vector<FileSystem::FileEntry> files;
        const std::filesystem::path relativePath = std::filesystem::path("Assets") / path;
        for (const auto &entry : std::filesystem::directory_iterator(relativePath))
        {
            if (entry.is_regular_file())
            {
                files.push_back({ FileSystem::Flag::File, entry.path().filename().string() });
            }
            else if (entry.is_directory())
            {
                files.push_back({ FileSystem::Flag::Directory, entry.path().filename().string() });
            }
        }
        return files;
    }

    /// opens a file on the specified path
    Id::Type Open(const std::filesystem::path &path, bool write, bool binary)
    {
        std::filesystem::path fullPath = mPath / path;
        if (!Exists(path))
        {
            std::ofstream file(fullPath, (binary ? std::ios::binary : (std::ios::openmode)0));
        }
        std::ios::openmode mode = std::ios::in | std::ios::out;
        if (write)
        {
            mode |= std::ios::trunc;
        }
        if (binary)
        {
            mode |= std::ios::binary;
        }
        std::fstream file(fullPath, mode);
        if (file.is_open())
        {
            Id::Type fileId = FileSystem::GenerateFileId();
            mOpenFiles.emplace(fileId, std::move(file));
            return fileId;
        }
        return Id::Invalid;
    }

    /// closes a file handle
    void Close(const Id::Type handle)
    {
        mOpenFiles.erase(handle);
    }

    /// reads bytes from a file handle
    size_t ReadBytes(const Id::Type handle, void *buffer, const size_t size)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return false;
        }
        std::fstream &file = it->second.mFileStream;
        file.read(static_cast<char *>(const_cast<void *>(buffer)), size);
        return file.gcount();
    }

    /// writes bytes to a file handle
    bool WriteBytes(const Id::Type handle, const void *buffer, const size_t size)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return false;
        }
        std::fstream &file = it->second.mFileStream;
        file.write(static_cast<const char *>(buffer), size);
        return true;
    }

    /// reads text from a file handle
    std::string ReadText(const Id::Type handle, int lineCount)
    {
        std::string serializedData;

        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return serializedData;
        }
        std::fstream &file = it->second.mFileStream;

        std::string line;
        if (lineCount == -1)
        {
            while (std::getline(file, line))
            {
                serializedData += std::move(line);
            }
        }
        else
        {
            while (std::getline(file, line) && lineCount--)
            {
                serializedData += std::move(line);
            }
        }

        return serializedData;
    }

    /// writes text to a file handle
    bool WriteText(const Id::Type handle, const std::string &text)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return false;
        }
        std::fstream &file = it->second.mFileStream;

        file << text;
        return true;
    }

    /// gets the current position in the file by handle
    size_t Tell(const Id::Type handle)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return 0;
        }
        return static_cast<size_t>(it->second.mFileStream.tellg());
    }

    /// seeks to a position in the file by handle
    void Seek(const Id::Type handle, const size_t position)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return;
        }
        it->second.mFileStream.seekg(static_cast<std::streamoff>(position), std::ios::beg);
    }

    /// gets the size of the file by handle
    size_t Size(const Id::Type handle)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return static_cast<size_t>(-1);
        }
        std::fstream &file = it->second.mFileStream;

        // get file size by seeking to the end and back
        const std::streampos current = file.tellg();
        file.seekg(0, std::ios::end);
        const std::streampos end = file.tellg();
        file.seekg(current, std::ios::beg);

        return static_cast<size_t>(end);
    }

private:
    /// file state struct
    struct FileState
    {
        /// file stream
        std::fstream mFileStream;
    };

    /// path
    const std::filesystem::path mPath;

    /// open files
    std::unordered_map<Id::Type, FileState> mOpenFiles;
};

//==============================================================================================================================================================================

/// constructs a folder file system
FolderFileSystem::FolderFileSystem(const std::filesystem::path &path) :
    IFileSystem(), mImpl(FolderFileSystem::Impl::MakeUniquePtr(FileSystem::NormalizeDirPath(path))) {}

/// creates a smart pointer version of the folder file system
IFileSystemPtr FolderFileSystem::MakePtr(const std::filesystem::path &path)
{
    return IFileSystemPtr(new FolderFileSystem(path));
}

/// initialize file system
void FolderFileSystem::Initialize()
{
    return mImpl->Initialize();
}

/// whether this file system is packed
bool FolderFileSystem::Packed() const
{
    return mImpl->Packed();
}

/// whether this file system handles the specified file handle
bool FolderFileSystem::Handles(Id::Type handle)
{
    return mImpl->Handles(handle);
}

/// check if a file exists
bool FolderFileSystem::Exists(const std::filesystem::path &path)
{
    return mImpl->Exists(path);
}

/// list files in a directory
std::vector<FileSystem::FileEntry> FolderFileSystem::ListFiles(const std::filesystem::path &path)
{
    return mImpl->ListFiles(path);
}

/// opens a file on the specified path
Id::Type FolderFileSystem::Open(const std::filesystem::path &path, bool write, bool binary)
{
    return mImpl->Open(path, write, binary);
}

/// closes a file handle
void FolderFileSystem::Close(const Id::Type handle)
{
    mImpl->Close(handle);
}

/// reads bytes from a file handle
size_t FolderFileSystem::ReadBytes(const Id::Type handle, void *buffer, const size_t size)
{
    return mImpl->ReadBytes(handle, buffer, size);
}

/// writes bytes to a file handle
bool FolderFileSystem::WriteBytes(const Id::Type handle, const void *buffer, const size_t size)
{
    return mImpl->WriteBytes(handle, buffer, size);
}

/// reads text from a file handle
std::string FolderFileSystem::ReadText(const Id::Type handle, const int lineCount)
{
    return mImpl->ReadText(handle, lineCount);
}

/// writes text to a file handle
bool FolderFileSystem::WriteText(const Id::Type handle, const std::string &text)
{
    return mImpl->WriteText(handle, text);
}

/// gets the current position in the file by handle
size_t FolderFileSystem::Tell(const Id::Type handle)
{
    return mImpl->Tell(handle);
}

/// seeks to a position in the file by handle
void FolderFileSystem::Seek(const Id::Type handle, const size_t position)
{
    mImpl->Seek(handle, position);
}

/// gets the size of the file by handle
size_t FolderFileSystem::Size(const Id::Type handle)
{
    return mImpl->Size(handle);
}
