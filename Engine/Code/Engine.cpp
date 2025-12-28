//==============================================================================================================================================================================
/// \file
/// \brief     engine extra code
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngine.h"
#include "lSceneManager.h"
#include "lFileSystemResources.h"
#include "lBuiltinResources.h"

#include "EngineImpl.h"

using namespace Lumen;

/// constructor
Engine::Engine(const ApplicationPtr &application, Impl *impl) : mApplication(application), mImpl(ImplUniquePtr(impl)) {}

/// get implementation
Engine::Impl *Engine::GetImpl() const
{
    return mImpl.get();
}

/// get application
ApplicationWeakPtr Engine::GetApplication() const
{
    return mApplication;
}

/// initialization and management
bool Engine::Initialize(const Object &config)
{
    AssetManager::Initialize(shared_from_this());
    AssetManager::RegisterFactory(FileSystemResources::MakePtr(1.0f));
    AssetManager::RegisterFactory(BuiltinResources::MakePtr(0.1f));

    FileSystem::Initialize();
    SceneManager::Initialize();

    if (!mApplication)
        return false;

#ifdef EDITOR
    DebugLog::Info("Engine initialized in editor mode");
#endif

    // initialize application
    mApplication->Initialize(GetApplication());
    if (!mImpl->Initialize(config))
    {
        return false;
    }

    // process initial detected files
    std::list<std::vector<Lumen::AssetManager::AssetChange>> batchQueue;
    if (PopAssetChangeBatchQueue(batchQueue))
    {
        AssetManager::ProcessAssetChanges(std::move(batchQueue));
    }

    // success
    return true;
}

/// shutdown
void Engine::Shutdown()
{
    // shutdown application
    if (mApplication)
        mApplication->Shutdown();

    SceneManager::Shutdown();
    FileSystem::Shutdown();

    AssetManager::Shutdown();

    mImpl->Shutdown();
}

/// new project
bool Engine::New()
{
    mApplication->New();
    return true;
}

/// open project
bool Engine::Open()
{
    mApplication->Open();
    return mImpl->CreateNewResources();
}

/// basic game loop
bool Engine::Run()
{
    // process file changes
    std::list<std::vector<Lumen::AssetManager::AssetChange>> batchQueue;
    if (PopAssetChangeBatchQueue(batchQueue))
    {
        AssetManager::ProcessAssetChanges(std::move(batchQueue));
    }

    // run application
    if (mApplication)
    {
        return mImpl->Run(std::function<bool()>([&]() { return mApplication->Run(mImpl->GetElapsedTime()); }),
#ifdef EDITOR
            std::function<void()>([&]() { mApplication->Editor(); }));
#else
            nullptr);
#endif
    }

    return false;
}

#ifdef EDITOR
/// get settings
Engine::Settings Engine::GetSettings() const
{
    return mImpl->GetSettings();
}

/// set settings
void Engine::SetSettings(Engine::Settings &settings)
{
    mImpl->SetSettings(settings);
}
#endif

/// get fullscreen size
void Engine::GetFullscreenSize(int &width, int &height) const
{
    return mImpl->GetFullscreenSize(width, height);
}

/// create a file system for the assets
IFileSystemPtr Engine::AssetsFileSystem() const
{
    return mImpl->AssetsFileSystem();
}

/// push a batch of asset changes (monitoring)
void Engine::PushAssetChangeBatch(std::vector<AssetManager::AssetChange> &&batch)
{
    mImpl->PushAssetChangeBatch(std::move(batch));
}

/// pop all batches of items
bool Engine::PopAssetChangeBatchQueue(std::list<std::vector<AssetManager::AssetChange>> &batchQueue)
{
    return mImpl->PopAssetChangeBatchQueue(batchQueue);
}

/// begin scene
void Engine::BeginScene()
{
    return mImpl->BeginScene();
}

/// push render command
void Engine::PushRenderCommand(Engine::RenderCommand renderCommand)
{
    return mImpl->PushRenderCommand(renderCommand);
}

/// end scene
void Engine::EndScene()
{
    return mImpl->EndScene();
}

/// create a texture
Id::Type Engine::CreateTexture(const TexturePtr &texture, int width, int height)
{
    return mImpl->CreateTexture(texture, width, height);
}

/// release a texture
void Engine::ReleaseTexture(Id::Type texId)
{
    mImpl->ReleaseTexture(texId);
}

/// create a shader
Id::Type Engine::CreateShader(const ShaderPtr &shader)
{
    return mImpl->CreateShader(shader);
}

/// release a shader
void Engine::ReleaseShader(Id::Type shaderId)
{
    return mImpl->ReleaseShader(shaderId);
}

/// create a mesh
Id::Type Engine::CreateMesh(const MeshPtr &mesh)
{
    return mImpl->CreateMesh(mesh);
}

/// release a mesh
void Engine::ReleaseMesh(Id::Type meshId)
{
    mImpl->ReleaseMesh(meshId);
}

/// set render texture size
void Engine::SetRenderTextureSize(Id::Type texId, Math::Int2 size)
{
    mImpl->SetRenderTextureSize(texId, size);
}

/// get render texture id
qword Engine::GetRenderTextureHandle(Id::Type texId)
{
    return mImpl->GetRenderTextureHandle(texId);
}

//==============================================================================================================================================================================

#ifdef TYPEINFO
/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    /// function-local static to avoid static initialization order issues
    inline std::unordered_map<Hash, std::string> &GetTypeHashRegistry()
    {
        static std::unordered_map<Hash, std::string> registry;
        return registry;
    }

    /// hash (FNV-1a) name from current type, typeinfo version
    void RegisterTypeHash(Hash hash, std::string_view name)
    {
        auto &registry = GetTypeHashRegistry();
        auto it = registry.find(hash);
        if (it != registry.end())
        {
            L_ASSERT_MSG(it->second == name, "Hash collision detected for type names '{}' and '{}'", it->second, name);
        }
        else
        {
            registry[hash] = name;
        }
    }
}

/// hash (FNV-1a) type from type name, typeinfo version
HashType Lumen::EncodeType(const char *typeName)
{
    size_t end = std::string_view(typeName).size();
    Hash hash = HASH_OFFSET;
    size_t hashPos = 0;
    while (hashPos < end)
    {
        hash ^= *(typeName + hashPos);
        hash *= HASH_PRIME;
        hashPos++;
    }
    std::string_view name(typeName, end);
    Hidden::RegisterTypeHash(hash, name);
    return HashType(hash, name);
}

/// hash (FNV-1a) type from current function name, typeinfo version
HashType Lumen::ClassType(const char *currentFunction)
{
    size_t end = std::string_view(currentFunction).find_last_of('(');
    end = std::string_view(currentFunction, end).find_last_of(':') - 1;
    size_t begin = std::string_view(currentFunction, end).find_last_of(' ') + 1;
    Hash hash = HASH_OFFSET;
    size_t hashPos = begin;
    while (hashPos < end)
    {
        hash ^= *(currentFunction + hashPos);
        hash *= HASH_PRIME;
        hashPos++;
    }
    std::string_view name(currentFunction + begin, end - begin);
    Hidden::RegisterTypeHash(hash, name);
    return HashType(hash, name);
}
#endif
