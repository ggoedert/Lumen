//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include "lCamera.h"

bool MainScene::Load()
{
    mPlayer = Lumen::GameObject::MakePtr();
    Lumen::DebugLog::Info("MainScene::Load");

    mPlayer = Lumen::GameObject::MakePtr();
    mCamera = Lumen::GameObject::MakePtr();

    if (auto gameObjectLock = mCamera.lock())
    {
        gameObjectLock->AddComponent(Lumen::Camera::Type(), Lumen::Camera::Params({ mCamera, {0.4509f, 0.8431f, 1.f, 1.f} }));

        Lumen::Math::Vector col = mApplication.GetBackgroundColor();

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
