//==============================================================================================================================================================================
/// \file
/// \brief     file interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(IFileSystem);

    /// FileSystem interface class
    class IFileSystem
    {
        CLASS_NO_COPY_MOVE(IFileSystem);

    public:
        /// opens a file on the specified path
        virtual void Open(std::string_view path) const = 0;

    protected:
        /// default constructor
        explicit IFileSystem() = default;

        /// pure virtual destructor
        virtual ~IFileSystem() = default;
    };

    /// FileSystem namespace
    namespace FileSystem
    {
        /// initialize file namespace
        void Initialize();

        /// shutdown file namespace
        void Shutdown();

        /// register a file system
        void RegisterFileSystem(const IFileSystemPtr &fileSystem);

        /// opens a file on the specified path
        void Open(std::string_view path);
    };
}
