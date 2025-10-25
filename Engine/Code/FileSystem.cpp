//==============================================================================================================================================================================
/// \file
/// \brief     file
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFileSystem.h"

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
        std::vector<IFileSystemPtr> mFileSystems;
    };

    static std::unique_ptr<FileState> gFileState;
}

/// checks if a path starts with a prefix
bool IFileSystem::StartsWith(const std::filesystem::path &path, const std::filesystem::path &prefix)
{
    for (auto itPath = path.begin(), itPrefix = prefix.begin(); itPrefix != prefix.end(); ++itPath, ++itPrefix)
    {
        if (itPath == path.end() || *itPath != *itPrefix)
            return false;
    }
    return true;
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

/// register file system
void FileSystem::RegisterFileSystem(const IFileSystemPtr &fileSystem)
{
    L_ASSERT(Hidden::gFileState);
    Hidden::gFileState->mFileSystems.push_back(fileSystem);
}

/// opens a file on the specified path
void FileSystem::Open(const std::filesystem::path &path)
{
}
