//==============================================================================================================================================================================
/// \file
/// \brief     Engine interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lFileSystem.h"
#include "lApplication.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Application);
    CLASS_PTR_DEF(Texture);

    /// Engine class
    class Engine : public std::enable_shared_from_this<Engine>
    {
        CLASS_NO_DEFAULT_CTOR(Engine);
        CLASS_NO_COPY_MOVE(Engine);
        class Impl;

    public:
        /// texture id type
        using TextureID = size_t;

        /// invalid texture id
        static constexpr TextureID InvalidTextureID = static_cast<TextureID>(SIZE_MAX);

        /// destructor
        ~Engine() noexcept = default;

        /// allocate smart pointer version of the engine, implemented at platform level
        static EnginePtr MakePtr(const ApplicationPtr &application);

        /// debug log, implemented at platform level
        static void DebugOutput(const std::string &message);

        /// initialization and management
        bool Initialize(const Object &config);

        /// shutdown
        void Shutdown();

        /// basic game loop
        bool Run();

        /// messages
        void OnActivated();
        void OnDeactivated();
        void OnSuspending();
        void OnResuming();
        void OnWindowMoved();
        void OnDisplayChange();
        void OnWindowSizeChanged(int width, int height);

        /// properties
        void GetDefaultSize(int &width, int &height) const;

        /// create a folder file system
        IFileSystemPtr FolderFileSystem(std::string_view name, std::string_view path) const;

        /// register a texture
        TextureID RegisterTexture(const TexturePtr &texture, int width, int height);

        /// unregister a texture
        void UnregisterTexture(TextureID texID);

    protected:
        /// protected constructor
        explicit Engine(const ApplicationPtr &application, Impl *impl);

        // application
        ApplicationPtr mApplication;

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
