//==============================================================================================================================================================================
/// \file
/// \brief     client application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "Sandbox.h"

#include "lEngine.h"
#include "lSceneManager.h"

#include <fstream>

/// initialize sandbox
void Sandbox::Initialize(const Lumen::ApplicationWeakPtr &application)
{
    Lumen::Application::Initialize(application);
    if (auto engineLock = GetEngine().lock())
    {
        Lumen::FileSystem::RegisterFileSystem("Assets", engineLock->AssetsFileSystem());
    }
}

/// shutdown sandbox
void Sandbox::Shutdown()
{
    if (auto engineLock = GetEngine().lock())
    {
        engineLock->New();
    }
    Lumen::Application::Shutdown();
}

/// get winwdow size
void Sandbox::GetWindowSize(int &width, int &height)
{
    width = 1280;
    height = 720;
    if (auto engineLock = GetEngine().lock())
    {
        engineLock->GetFullscreenSize(width, height);
    }
    width /= 3;
    height /= 3;
}

/// TEMP DELME!
void Sandbox::New()
{
    mMainScene.reset();
}

/// TEMP DELME!
void Sandbox::Open()
{
    if (auto engineLock = GetEngine().lock())
    {
        engineLock->New();
    }
    mMainScene = Lumen::Scene::MakePtr(*this, "Assets/MainScene.lumen");
    if (!Lumen::SceneManager::Load(mMainScene))
    {
        Shutdown();
    }
    else
    {
        Lumen::Serialized::Type out;
        mMainScene->Serialize(out, false);
        std::ofstream("Assets\\serializer_test.txt") << out.dump(4);
    }
}
