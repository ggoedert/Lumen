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

    /// opens a file on the specified path
    Id::Type Open(const std::filesystem::path &path, const FileSystem::FileMode mode)
    {
        std::filesystem::path fullPath = mPath / path;
        if (mode == FileSystem::FileMode::Text)
        {
            std::ifstream file(fullPath);
            if (file.is_open())
            {
                Id::Type fileId = FileSystem::GenerateFileId();
                mOpenFiles.emplace(fileId, std::move(file));
                return fileId;
            }
        }
        else
        {
            //@REVEW@ !dome!
        }
        return Id::Invalid;
    }

    /// closes a file handle
    void Close(const Id::Type handle)
    {
        mOpenFiles.erase(handle);
    }

    /// reads bytes from a file handle
    size_t ReadBytes(const Id::Type handle, const void *buffer, const size_t size)
    {
        return 0;
    }

    /// reads a line from a file handle
    std::vector<std::string> ReadLines(const Id::Type handle, int lineCount = -1)
    {
        std::vector<std::string> lines;

        auto it = mOpenFiles.find(handle);
        if (it == mOpenFiles.end())
        {
            return lines;
        }
        std::ifstream &file = it->second;

        std::string line;
        if (lineCount == -1)
        {
            while (std::getline(file, line))
            {
                lines.push_back(std::move(line));
            }
        }
        else
        {
            while (std::getline(file, line) && lineCount--)
            {
                lines.push_back(std::move(line));
            }
        }
            
        return lines;
    }

    /// gets the current position in the file by handle
    size_t Tell(const Id::Type handle)
    {
        return 0;
    }

    /// seeks to a position in the file by handle
    void Seek(const Id::Type handle, const size_t position)
    {
    }

    /// gets the size of the file by handle
    size_t Size(const Id::Type handle)
    {
        return 0;
    }

private:
    /// path
    const std::filesystem::path mPath;

    /// open files
    std::unordered_map<Id::Type, std::ifstream> mOpenFiles;

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

/// opens a file on the specified path
Id::Type FolderFileSystem::Open(const std::filesystem::path &path, const FileSystem::FileMode mode)
{
    return mImpl->Open(path, mode);
}

/// closes a file handle
void FolderFileSystem::Close(const Id::Type handle)
{
    mImpl->Close(handle);
}

/// reads bytes from a file handle
size_t FolderFileSystem::ReadBytes(const Id::Type handle, const void *buffer, const size_t size)
{
    return mImpl->ReadBytes(handle, buffer, size);
}

/// reads a line from a file handle
std::vector<std::string> FolderFileSystem::ReadLines(const Id::Type handle, const int lines)
{
    return mImpl->ReadLines(handle, lines);
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
