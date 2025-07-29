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
    CLASS_NO_DEFAULT_CTOR(FolderFileSystem::Impl);
    CLASS_NO_COPY_MOVE(FolderFileSystem::Impl);
    CLASS_PTR_UNIQUEMAKER(FolderFileSystem::Impl);
    friend class FolderFileSystem;

public:
    /// constructs a folder file system implementation
    explicit Impl(std::string_view name, std::string_view path) : mName(name), mPath(path) {}

    /// opens a file on the specified path
    void Open(std::string_view path) const {}

private:
    /// name
    const std::string mName;

    /// path
    const std::string mPath;
};

//==============================================================================================================================================================================

/// constructs a folder file system
FolderFileSystem::FolderFileSystem(std::string_view name, std::string_view path) :
    IFileSystem(), mImpl(FolderFileSystem::Impl::MakeUniquePtr(name, path)) {}

/// creates a smart pointer version of the folder file system
IFileSystemPtr FolderFileSystem::MakePtr(std::string_view name, std::string_view path)
{
    return IFileSystemPtr(new FolderFileSystem(name, path));
}

/// opens a file on the specified path
void FolderFileSystem::Open(std::string_view path) const
{
    return mImpl->Open(path);
}
