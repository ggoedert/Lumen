//==============================================================================================================================================================================
/// \file
/// \brief     engine extra code
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEngine.h"
#include "lSceneManager.h"
#include "lFileSystem.h"

using namespace Lumen;

// initialization and management
bool Engine::Initialize(const std::any &config)
{
    Lumen::FileSystem::Initialize();
    Lumen::SceneManager::Initialize();

    if (!mApplication)
        return false;

    mApplication->SetEngine(shared_from_this());

    // initialize application
    return mApplication->Initialize();
}

/// shutdown
void Engine::Shutdown()
{
    // shutdown application
    if (mApplication)
        mApplication->Shutdown();

    Lumen::SceneManager::Shutdown();
    Lumen::FileSystem::Shutdown();
}

/// run engine
bool Engine::Run(float deltaTime)
{
    if (!mApplication)
        return false;

    // run application
    return mApplication->Run(deltaTime);
}

//==============================================================================================================================================================================

#ifdef _DEBUG
/// hash (FNV-1a) class name from current function name, debug version
HashType Lumen::ClassNameType(const char *currentFunction)
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
    return HashType(hash, std::string_view(currentFunction + begin, end - begin));
}
#endif
