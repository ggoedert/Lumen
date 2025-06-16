//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include "lCamera.h"

bool MainScene::Load()
{
    Lumen::DebugLog::Info("MainScene::Load");

    if (auto gameObjectLock = mCamera.lock())
    {
        gameObjectLock->AddComponent(Lumen::Camera::Type(), Lumen::Camera::Params({ {0.4509f, 0.8431f, 1.f, 1.f} }));

        Lumen::Math::Vector col = mApplication.BackgroundColor();

        Lumen::DebugLog::Info("Static component name: " + Lumen::Camera::Name());
        if (auto cameraComponentLock = gameObjectLock->Component(Lumen::Camera::Type()).lock())
        {
            Lumen::DebugLog::Info("Polymorphic component instance name: " + cameraComponentLock->Name());
        }
    }
    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mPlayer.reset();
}
