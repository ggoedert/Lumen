//==============================================================================================================================================================================
/// \file
/// \brief     client application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "Sandbox.h"

#include "lEngine.h"
#include "lSceneManager.h"

/// initialize sandbox
void Sandbox::Initialize(const Lumen::ApplicationWeakPtr &application)
{
    Lumen::Application::Initialize(application);
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

    // Scene::Load reads the Library entry when it is valid, otherwise the source in Assets (which then refreshes the Library),
    // the source is only written when the scene is saved
    mMainScene = Lumen::Scene::MakePtr(*this, "Assets/MainScene.lumen");
    if (!Lumen::SceneManager::Load(mMainScene))
    {
        Lumen::DebugLog::Error("Unable to load Assets/MainScene.lumen");
        Shutdown();
    }
}
