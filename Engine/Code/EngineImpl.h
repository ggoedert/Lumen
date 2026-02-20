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

        /// set owner
        virtual void SetOwner(EngineWeakPtr owner) = 0;

        /// initialization and management
        virtual bool Initialize(const Object &config) = 0;

#ifdef EDITOR
        /// check if initialized
        virtual bool Initialized() = 0;
#endif

        /// create new resources
        virtual bool CreateNewResources() = 0;

        /// shutdown
        virtual void Shutdown() = 0;

        /// get elapsed time since last run
        virtual float GetElapsedTime() = 0;

        /// basic game loop
        virtual bool Run(std::function<bool()> update, std::function<void()> preRender) = 0;

#ifdef EDITOR
        /// get executable name
        virtual std::string GetExecutableName() const = 0;

        /// get settings
        virtual Engine::Settings GetSettings() = 0;

        /// set settings
        virtual void SetSettings(Engine::Settings &settings) = 0;

        /// check if light theme is used
        virtual bool IsLightTheme() const = 0;
#endif

        /// get fullscreen size
        virtual void GetFullscreenSize(int &width, int &height) const = 0;

        /// create a folder file system
        virtual IFileSystemPtr AssetsFileSystem() const = 0;

        /// post event
        virtual void PostEvent(EventUniquePtr event) = 0;

        /// post render command
        virtual void PostRenderCommand(RenderCommandUniquePtr renderCommand) = 0;

        /// create a texture
        virtual Id::Type CreateTexture(const TexturePtr &texture, int width, int height) = 0;

        /// release a texture
        virtual void ReleaseTexture(Id::Type texId) = 0;

        /// create a shader
        virtual Id::Type CreateShader(const ShaderPtr &shader) = 0;

        /// release a shader
        virtual void ReleaseShader(Id::Type shaderId) = 0;

        /// create a mesh
        virtual Id::Type CreateMesh(const MeshPtr &mesh) = 0;

        /// release a mesh
        virtual void ReleaseMesh(Id::Type meshId) = 0;

        /// set render texture size
        virtual void SetRenderTextureSize(Id::Type texId, Math::Int2 size) = 0;

        /// get render texture id
        virtual qword GetRenderTextureHandle(Id::Type texId) = 0;

        /// pop all batches of items
        virtual bool PopAssetChangeBatchQueue(std::list<std::vector<AssetManager::AssetChange>> &batchQueue) = 0;
    };
}
