//==============================================================================================================================================================================
/// \file
/// \brief     scene manager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lSceneManager.h"

using namespace Lumen;

/// load scene
bool SceneManager::Load(Scene::Ptr scene)
{
    //unload last scene
    Unload();

    //load current scene
    mCurrentScene = std::move(scene);
    return mCurrentScene->Load();
}

/// unload current scene
void SceneManager::Unload()
{
    if (mCurrentScene)
    {
        mCurrentScene->Unload();
        mCurrentScene.reset();
    }
}
