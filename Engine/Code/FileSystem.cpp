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

/// initialize file namespace
void FileSystem::Initialize()
{
    LUMEN_ASSERT(!Hidden::gFileState);
    Hidden::gFileState = std::make_unique<Hidden::FileState>();
}

/// shutdown file namespace
void FileSystem::Shutdown()
{
    LUMEN_ASSERT(Hidden::gFileState);
    Hidden::gFileState.reset();
}

/// register file system
void FileSystem::RegisterFileSystem(const IFileSystemPtr &fileSystem)
{
    LUMEN_ASSERT(Hidden::gFileState);
    Hidden::gFileState->mFileSystems.push_back(fileSystem);
}

/// opens a file on the specified path
void FileSystem::Open(std::string_view path) {}
