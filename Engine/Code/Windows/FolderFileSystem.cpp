//==============================================================================================================================================================================
/// \file
/// \brief     FolderFileSystem
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFolderFileSystem.h"

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
    explicit Impl(std::string_view name, const std::filesystem::path &path) : mName(name), mPath(path) {}

    /// opens a file on the specified path
    void Open(const std::filesystem::path &path) const {}

private:
    /// name
    const std::string mName;

    /// path
    const std::filesystem::path mPath;
};

//==============================================================================================================================================================================

/// constructs a folder file system
FolderFileSystem::FolderFileSystem(std::string_view name, const std::filesystem::path &path) :
    IFileSystem(), mImpl(FolderFileSystem::Impl::MakeUniquePtr(name, path)) {}

/// creates a smart pointer version of the folder file system
IFileSystemPtr FolderFileSystem::MakePtr(std::string_view name, const std::filesystem::path &path)
{
    return IFileSystemPtr(new FolderFileSystem(name, path));
}

/// opens a file on the specified path
void FolderFileSystem::Open(const std::filesystem::path &path) const
{
    return mImpl->Open(path);
}
