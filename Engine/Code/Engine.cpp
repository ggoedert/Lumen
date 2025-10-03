//==============================================================================================================================================================================
/// \file
/// \brief     engine extra code
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngine.h"
#include "lSceneManager.h"
#include "lBuiltinResources.h"

#include "EngineImpl.h"

using namespace Lumen;

/// constructor
Engine::Engine(const ApplicationPtr &application, Impl *impl) : mApplication(application), mImpl(ImplUniquePtr(impl)) {}

/// initialization and management
bool Engine::Initialize(const Object &config)
{
    Assets::Initialize(shared_from_this());
    Assets::RegisterFactory(DefaultResources::MakePtr(), 1.0f);
    Assets::RegisterFactory(BuiltinExtra::MakePtr(), 1.0f);

    FileSystem::Initialize();
    SceneManager::Initialize();

    if (!mApplication)
        return false;

    mApplication->SetEngine(shared_from_this());

#ifdef EDITOR
    DebugLog::Info("Engine initialized in editor mode");
#endif

    // initialize application
    if (!mApplication->Initialize())
        return false;

    return mImpl->Initialize(config);
}

/// shutdown
void Engine::Shutdown()
{
    // shutdown application
    if (mApplication)
        mApplication->Shutdown();

    SceneManager::Shutdown();
    FileSystem::Shutdown();

    Assets::Shutdown();

    mImpl->Shutdown();
}

/// basic game loop
bool Engine::Run()
{
    // run application
    if (mApplication && mApplication->Run(mImpl->GetElapsedTime()))
        return mImpl->Run();

    return false;
}

/// messages
void Engine::OnActivated() { mImpl->OnActivated(); }
void Engine::OnDeactivated() { mImpl->OnDeactivated(); }
void Engine::OnSuspending() { mImpl->OnSuspending(); }
void Engine::OnResuming() { mImpl->OnResuming(); }
void Engine::OnWindowMoved() { mImpl->OnWindowMoved(); }
void Engine::OnDisplayChange() { mImpl->OnDisplayChange(); }
void Engine::OnWindowSizeChanged(int width, int height) { mImpl->OnWindowSizeChanged(width, height); }

/// properties
void Engine::GetDefaultSize(int &width, int &height) const
{
    return mImpl->GetDefaultSize(width, height);
}

/// create a folder file system
IFileSystemPtr Engine::FolderFileSystem(std::string_view name, std::string_view path) const
{
    return mImpl->FolderFileSystem(name, path);
}

/// register a texture
Engine::IdType Engine::RegisterTexture(const TexturePtr &texture, int width, int height)
{
    return mImpl->RegisterTexture(texture, width, height);
}

/// unregister a texture
void Engine::UnregisterTexture(IdType texID)
{
    mImpl->UnregisterTexture(texID);
}

//==============================================================================================================================================================================

#ifdef TYPEINFO

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    static std::unordered_map<Hash, std::string> gTypeHashRegistry;

    /// hash (FNV-1a) name from current type, typeinfo version
    void RegisterTypeHash(Hash hash, std::string_view name)
    {
        auto it = gTypeHashRegistry.find(hash);
        if (it != gTypeHashRegistry.end())
        {
            L_ASSERT_MSG(it->second == name, "Hash collision detected for type names '{}' and '{}'", it->second, name);
        }
        else
        {
            gTypeHashRegistry[hash] = name;
        }
    }
}

/// hash (FNV-1a) name from current type, typeinfo version
HashType Lumen::PodType(const char *currentType)
{
    size_t end = std::string_view(currentType).size();
    Hash hash = HASH_OFFSET;
    size_t hashPos = 0;
    while (hashPos < end)
    {
        hash ^= *(currentType + hashPos);
        hash *= HASH_PRIME;
        hashPos++;
    }
    std::string_view name(currentType, end);
    Hidden::RegisterTypeHash(hash, name);
    return HashType(hash, name);
}

/// hash (FNV-1a) class name from current function name, typeinfo version
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
