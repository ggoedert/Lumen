//==============================================================================================================================================================================
/// \file
/// \brief     Engine interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lId.h"
#include "SimpleMath.h"
#include "lFileSystem.h"
#include "lApplication.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Application);
    CLASS_PTR_DEF(Texture);
    CLASS_PTR_DEF(Shader);
    CLASS_PTR_DEF(Mesh);

    /// Engine class
    class Engine : public std::enable_shared_from_this<Engine>
    {
        CLASS_NO_DEFAULT_CTOR(Engine);
        CLASS_NO_COPY_MOVE(Engine);
        class Impl;

    public:
        struct DrawPrimitive
        {
            Id::Type meshId;
            Id::Type shaderId;
            Id::Type texId;
            DirectX::SimpleMath::Matrix world;
        };
        using RenderCommand = std::variant<DrawPrimitive>;

        /// destructor
        ~Engine() = default;

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
        IFileSystemPtr FolderFileSystem(std::string_view name, const std::filesystem::path &path) const;

        /// begin scene
        void BeginScene();

        /// push render command
        void PushRenderCommand(RenderCommand renderCommand);

        /// end scene
        void EndScene();

        /// create a texture
        Id::Type CreateTexture(const TexturePtr &texture, int width, int height);

        /// release a texture
        void ReleaseTexture(Id::Type texId);

        /// create a shader
        Id::Type CreateShader(const ShaderPtr &shader);

        /// release a shader
        void ReleaseShader(Id::Type shaderId);

        /// create a mesh
        Id::Type CreateMesh(const MeshPtr &mesh);

        /// release a mesh
        void ReleaseMesh(Id::Type meshId);

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
