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
    explicit Impl(const std::string &name, const std::string &path) : mName(name), mPath(path) {}

    /// opens a file on the specified path
    void Open(const std::string &path) const {}

private:
    /// name
    const std::string &mName;

    /// path
    const std::string &mPath;
};

//==============================================================================================================================================================================

/// constructs a folder file system
FolderFileSystem::FolderFileSystem(const std::string &name, const std::string &path) :
    IFileSystem(), mImpl(FolderFileSystem::Impl::MakeUniquePtr(name, path)) {}

/// creates a smart pointer version of the folder file system
IFileSystemPtr FolderFileSystem::MakePtr(const std::string &name, const std::string &path)
{
    return IFileSystemPtr(new FolderFileSystem(name, path));
}

/// opens a file on the specified path
void FolderFileSystem::Open(const std::string &path) const
{
    return mImpl->Open(path);
}
