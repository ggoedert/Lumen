//==============================================================================================================================================================================
/// \file
/// \brief     engine extra code
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngine.h"
#include "lSceneManager.h"
#include "lFileSystemResources.h"
#include "lBuiltinResources.h"

#include "EnginePlatform.h"

using namespace Lumen;

/// Engine::Impl class
class Engine::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructor
    explicit Impl(EnginePlatform *platform, const ApplicationPtr &application) : mPlatform(EnginePlatformPtr(platform)), mApplication(application) {}

    /// destructor
    ~Impl() = default;

    /// set owner
    void SetOwner(EngineWeakPtr owner)
    {
        mOwner = owner;
        mPlatform->SetOwner(mOwner);
        mApplication->SetEngine(mOwner);
    }

    /// get application
    ApplicationWeakPtr GetApplication() const
    {
        return mApplication;
    }

    /// initialization and management
    bool Initialize(const Object &config)
    {
        //AssetManager::Initialize(shared_from_this());
        AssetManager::Initialize(mOwner);
        AssetManager::RegisterFactory(FileSystemResources::MakePtr(1.0f));
        AssetManager::RegisterFactory(BuiltinResources::MakePtr(0.1f));

        FileSystem::Initialize();
        SceneManager::Initialize();

        if (!mApplication)
            return false;

#ifdef EDITOR
        using clock = std::chrono::system_clock;
        auto time = clock::to_time_t(clock::now());
        std::tm timeInfo;
        localtime_s(&timeInfo, &time);
        std::ostringstream oss;
        oss << std::put_time(&timeInfo, "%Y-%m-%d");
        DebugLog::Info("[{}] Engine initialized in editor mode", oss.str());
#endif

        // initialize application
        mApplication->Initialize(GetApplication());
        if (!mPlatform->Initialize(config))
        {
            return false;
        }

        // process initial detected files
        std::list<std::vector<Lumen::AssetManager::AssetChange>> batchQueue;
        if (mPlatform->PopAssetChangeBatchQueue(batchQueue))
        {
            AssetManager::ProcessAssetChanges(std::move(batchQueue));
        }

        // success
        return true;
    }

#ifdef EDITOR
    /// check if initialized
    bool Initialized()
    {
        return mPlatform->Initialized();
    }
#endif

    /// shutdown
    void Shutdown()
    {
        // shutdown application
        if (mApplication)
            mApplication->Shutdown();

        SceneManager::Shutdown();
        FileSystem::Shutdown();

        AssetManager::Shutdown();

        mPlatform->Shutdown();
    }

    /// new project
    bool New()
    {
        mApplication->New();
        return true;
    }

    /// open project
    bool Open()
    {
        mApplication->Open();
        return mPlatform->CreateNewResources();
    }

    /// basic game loop
    bool Run()
    {
        // process file changes
        std::list<std::vector<Lumen::AssetManager::AssetChange>> batchQueue;
        if (mPlatform->PopAssetChangeBatchQueue(batchQueue))
        {
            AssetManager::ProcessAssetChanges(std::move(batchQueue));
        }

        // run application
        if (mApplication)
        {
            return mPlatform->Run(std::function<bool()>([&]() { return mApplication->Run(mPlatform->GetElapsedTime()); }),
#ifdef EDITOR
                std::function<void()>([&]() { mApplication->Editor(); }));
#else
                nullptr);
#endif
        }

        return false;
    }

#ifdef EDITOR
    /// get executable name
    std::string GetExecutableName() const
    {
        return mPlatform->GetExecutableName();
    }

    /// get settings
    Settings GetSettings() const
    {
        return mPlatform->GetSettings();
    }

    /// set settings
    void SetSettings(Settings &settings)
    {
        mPlatform->SetSettings(settings);
    }

    /// check if light theme is used
    bool IsLightTheme() const
    {
        return mPlatform->IsLightTheme();
    }
#endif

    /// get fullscreen size
    void GetFullscreenSize(int &width, int &height) const
    {
        return mPlatform->GetFullscreenSize(width, height);
    }

    /// create a file system for the assets
    IFileSystemPtr AssetsFileSystem() const
    {
        return mPlatform->AssetsFileSystem();
    }

    /// post event
    void PostEvent(EventUniquePtr event)
    {
        return mPlatform->PostEvent(std::move(event));
    }

    /// post render command
    void PostRenderCommand(RenderCommandUniquePtr renderCommand)
    {
        return mPlatform->PostRenderCommand(std::move(renderCommand));
    }

    /// create a texture
    Id::Type CreateTexture(const TexturePtr &texture, int width, int height)
    {
        return mPlatform->CreateTexture(texture, width, height);
    }

    /// release a texture
    void ReleaseTexture(Id::Type texId)
    {
        mPlatform->ReleaseTexture(texId);
    }

    /// create a shader
    Id::Type CreateShader(const ShaderPtr &shader)
    {
        return mPlatform->CreateShader(shader);
    }

    /// release a shader
    void ReleaseShader(Id::Type shaderId)
    {
        return mPlatform->ReleaseShader(shaderId);
    }

    /// create a mesh
    Id::Type CreateMesh(const MeshPtr &mesh)
    {
        return mPlatform->CreateMesh(mesh);
    }

    /// release a mesh
    void ReleaseMesh(Id::Type meshId)
    {
        mPlatform->ReleaseMesh(meshId);
    }

    /// set render texture size
    void SetRenderTextureSize(Id::Type texId, Math::Int2 size)
    {
        mPlatform->SetRenderTextureSize(texId, size);
    }

    /// get render texture id
    qword GetRenderTextureHandle(Id::Type texId)
    {
        return mPlatform->GetRenderTextureHandle(texId);
    }

private:
    /// owner
    EngineWeakPtr mOwner;

    /// platform virtual interface
    EnginePlatformPtr mPlatform;

    // application
    ApplicationPtr mApplication;
};


//==============================================================================================================================================================================

/// create a smart pointer version of the engine
EnginePtr Engine::MakePtr(EnginePlatform *platform, const ApplicationPtr &application)
{
    auto engine = EnginePtr(new Engine(platform, application));
    engine->mImpl->SetOwner(engine);
    return engine;
}

/// constructor
Engine::Engine(EnginePlatform *platform, const ApplicationPtr &application) : mImpl(Engine::Impl::MakeUniquePtr(platform, application)) {}

/// destructor
Engine::~Engine() = default;

/// get application
ApplicationWeakPtr Engine::GetApplication() const
{
    return mImpl->GetApplication();
}

/// initialization and management
bool Engine::Initialize(const Object &config)
{
    return mImpl->Initialize(config);
}

#ifdef EDITOR
/// check if initialized
bool Engine::Initialized()
{
    return mImpl->Initialized();
}
#endif

/// shutdown
void Engine::Shutdown()
{
    mImpl->Shutdown();
}

/// new project
bool Engine::New()
{
    return mImpl->New();
}

/// open project
bool Engine::Open()
{
    return mImpl->Open();
}

/// basic game loop
bool Engine::Run()
{
    return mImpl->Run();
}

#ifdef EDITOR
/// get executable name
std::string Engine::GetExecutableName() const
{
    return mImpl->GetExecutableName();
}

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

/// check if light theme is used
bool Engine::IsLightTheme() const
{
    return mImpl->IsLightTheme();
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

/// post event
void Engine::PostEvent(EventUniquePtr event)
{
    return mImpl->PostEvent(std::move(event));
}

/// post render command
void Engine::PostRenderCommand(RenderCommandUniquePtr renderCommand)
{
    return mImpl->PostRenderCommand(std::move(renderCommand));
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
