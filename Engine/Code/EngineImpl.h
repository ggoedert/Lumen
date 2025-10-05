//==============================================================================================================================================================================
/// \file
/// \brief     Engine private implementation virtual interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lEngine.h"

/// Lumen namespace
namespace Lumen
{
    /// Engine::Impl class
    class Engine::Impl
    {
        CLASS_NO_COPY_MOVE(Impl);
        CLASS_PTR_UNIQUEMAKER(Impl);

    public:
        /// constructs an engine
        explicit Impl() {}

        /// virtual destroys engine
        virtual ~Impl() = default;

        /// initialization and management
        virtual bool Initialize(const Object &config) = 0;

        /// shutdown
        virtual void Shutdown() = 0;

        /// get elapsed time since last run
        virtual float GetElapsedTime() = 0;

        /// basic game loop
        virtual bool Run() = 0;

        /// messages
        virtual void OnActivated() = 0;
        virtual void OnDeactivated() = 0;
        virtual void OnSuspending() = 0;
        virtual void OnResuming() = 0;
        virtual void OnWindowMoved() = 0;
        virtual void OnDisplayChange() = 0;
        virtual void OnWindowSizeChanged(int width, int height) = 0;

        /// properties
        virtual void GetDefaultSize(int &width, int &height) const = 0;

        /// create a folder file system
        virtual IFileSystemPtr FolderFileSystem(std::string_view name, std::string_view path) const = 0;

        /// create a texture
        virtual IdType CreateTexture(const TexturePtr &texture, int width, int height) = 0;

        /// release a texture
        virtual void ReleaseTexture(IdType texID) = 0;
    };
}
