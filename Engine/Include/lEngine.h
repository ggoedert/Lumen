//==============================================================================================================================================================================
/// \file
/// \brief     Engine interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lId.h"
#include "lFileSystem.h"
#include "lAsset.h"
#include "lApplication.h"
#include "lMath.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Application);
    CLASS_WEAK_PTR_DEF(Application);
    CLASS_PTR_DEF(Texture);
    CLASS_PTR_DEF(Shader);
    CLASS_PTR_DEF(Mesh);

    /// AssetChange struct forward declaration
    namespace AssetManager { struct AssetChange; }

    /// Engine class
    class Engine : public std::enable_shared_from_this<Engine>
    {
        CLASS_NO_DEFAULT_CTOR(Engine);
        CLASS_NO_COPY_MOVE(Engine);
        class Impl;

    public:
#ifdef EDITOR
        struct Settings
        {
            int posX = 0;
            int posY = 0;
            int width = 0;
            int height = 0;
            bool isMaximized = true;
            std::vector<std::string> imGuiIni;
        };
#endif

        struct DrawPrimitive
        {
            Id::Type meshId;
            Id::Type shaderId;
            Id::Type texId;
            Math::Matrix44 world;
        };
        using RenderCommand = std::variant<DrawPrimitive>;

        /// destructor
        ~Engine() = default;

        /// allocate smart pointer version of the engine, implemented at platform level
        [[nodiscard]] static EnginePtr MakePtr(const ApplicationPtr &application);

        /// get implementation
        [[nodiscard]] Impl *GetImpl() const;

        /// get application
        [[nodiscard]] ApplicationWeakPtr GetApplication() const;
        
        /// debug log, implemented at platform level
        static void DebugOutput(const std::string &message);

        /// initialization and management
        bool Initialize(const Object &config);

        /// shutdown
        void Shutdown();

        /// new project
        bool New();

        /// open project
        bool Open();

        /// basic game loop
        bool Run();

#ifdef EDITOR
        /// get executable name
        [[nodiscard]] std::string GetExecutableName() const;

        /// get windows settings
        [[nodiscard]] Settings GetSettings() const;

        /// set windows settings
        void SetSettings(Settings &settings);
#endif

        /// get fullscreen size
        void GetFullscreenSize(int &width, int &height) const;

        /// create a file system for the assets
        [[nodiscard]] IFileSystemPtr AssetsFileSystem() const;

        /// push a batch of asset changes (monitoring)
        void PushAssetChangeBatch(std::vector<AssetManager::AssetChange> &&batch);

        /// pop all batches of items
        bool PopAssetChangeBatchQueue(std::list<std::vector<AssetManager::AssetChange>> &batchQueue);

        /// begin scene
        void BeginScene();

        /// push render command
        void PushRenderCommand(RenderCommand renderCommand);

        /// end scene
        void EndScene();

        /// create a texture
        [[nodiscard]] Id::Type CreateTexture(const TexturePtr &texture, int width, int height);

        /// release a texture
        void ReleaseTexture(Id::Type texId);

        /// create a shader
        [[nodiscard]] Id::Type CreateShader(const ShaderPtr &shader);

        /// release a shader
        void ReleaseShader(Id::Type shaderId);

        /// create a mesh
        [[nodiscard]] Id::Type CreateMesh(const MeshPtr &mesh);

        /// release a mesh
        void ReleaseMesh(Id::Type meshId);

        /// set render texture size
        void SetRenderTextureSize(Id::Type texId, Math::Int2 size);

        /// get render texture id
        qword GetRenderTextureHandle(Id::Type texId);

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
