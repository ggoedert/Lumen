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
void Sandbox::Initialize()
{
    Lumen::Application::Initialize();
    if (auto engineLock = GetEngine().lock())
    {
        Lumen::FileSystem::RegisterFileSystem("Assets", engineLock->AssetsFileSystem());
    }
}

/// shutdown sandbox
void Sandbox::Shutdown()
{
    /// TEMP DELME!
    if (auto engineLock = GetEngine().lock())
    {
        engineLock->New();
    }
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
        //out["data"] = Serialized::Type::binary({ 0x01, 0x02, 0x03, 99, 100, 101, 127, 128, 129, 254, 255 });
        mMainScene->Serialize(out, false);
        std::ofstream("Assets\\serializer_test.txt") << out.dump(4);
    }

}
