//==============================================================================================================================================================================
/// \file
/// \brief     FolderFileSystem
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFolderFileSystem.h"

#include <fstream>
#include <unordered_map>

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

    /// whether this file system is packed
    bool Packed() const
    {
        return false;
    }

    /// whether this file system handles the specified file handle
    bool HandlesFileId(Id::Type handle)
    {
        return mOpenFiles.find(handle) != mOpenFiles.end();
    }

    /// check if a file exists
    bool Exists(const std::filesystem::path &path)
    {
        std::filesystem::path fullPath = mPath / path;
        return std::filesystem::exists(fullPath);
    }

    /// opens a file on the specified path
    Id::Type Open(const std::filesystem::path &path, bool binary)
    {
        std::filesystem::path fullPath = mPath / path;
        if (!Exists(path))
        {
            std::ofstream file(fullPath, (binary ? std::ios::binary : (std::ios::openmode)0));
        }
        std::fstream file(fullPath, std::ios::in | std::ios::out | (binary ? std::ios::binary : (std::ios::openmode)0));
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
        std::fstream &file = it->second;
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
        std::fstream &file = it->second;
        file.write(static_cast<const char *>(buffer), size);
        return true;
    }

    /// reads lines from a file handle
    std::string ReadLines(const Id::Type handle, int lineCount = -1)
    {
        std::string serializedData;

        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return serializedData;
        }
        std::fstream &file = it->second;

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

    /// writes lines to a file handle
    bool WriteLines(const Id::Type handle, const std::string &lines)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return false;
        }
        std::fstream &file = it->second;

        file << lines;
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
        return static_cast<size_t>(it->second.tellg());
    }

    /// seeks to a position in the file by handle
    void Seek(const Id::Type handle, const size_t position)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return;
        }
        it->second.seekg(static_cast<std::streamoff>(position), std::ios::beg);
    }

    /// gets the size of the file by handle
    size_t Size(const Id::Type handle)
    {
        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return static_cast<size_t>(-1);
        }
        std::fstream &file = it->second;

        // get file size by seeking to the end and back
        const std::streampos current = file.tellg();
        file.seekg(0, std::ios::end);
        const std::streampos end = file.tellg();
        file.seekg(current, std::ios::beg);

        return static_cast<size_t>(end);
    }

private:
    /// path
    const std::filesystem::path mPath;

    /// open files
    std::unordered_map<Id::Type, std::fstream> mOpenFiles;

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

/// whether this file system is packed
bool FolderFileSystem::Packed() const
{
    return mImpl->Packed();
}

/// whether this file system handles the specified file handle
bool FolderFileSystem::HandlesFileId(Id::Type handle)
{
    return mImpl->HandlesFileId(handle);
}

/// check if a file exists
bool FolderFileSystem::Exists(const std::filesystem::path &path)
{
    return mImpl->Exists(path);
}

/// opens a file on the specified path
Id::Type FolderFileSystem::Open(const std::filesystem::path &path, bool binary)
{
    return mImpl->Open(path, binary);
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

/// reads lines from a file handle
std::string FolderFileSystem::ReadLines(const Id::Type handle, const int lines)
{
    return mImpl->ReadLines(handle, lines);
}

/// writes lines to a file handle
bool FolderFileSystem::WriteLines(const Id::Type handle, const std::string &lines)
{
    return mImpl->WriteLines(handle, lines);
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
