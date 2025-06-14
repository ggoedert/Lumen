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

    auto cameraLock = mCamera.lock();
    cameraLock->AddComponent(Lumen::Camera::GetType(), Lumen::Camera::Params({mCamera, {0.4509f, 0.8431f, 1.f, 1.f}}));

    Lumen::Math::Vector col = mApplication.GetBackgroundColor();

    Lumen::DebugLog::Info("static Camera component name: " + Lumen::Camera::Name());
    auto cameraComponent = cameraLock->Component(Lumen::Camera::GetType()).lock();
    if (cameraComponent)
    {
        Lumen::DebugLog::Info("polymorphic Component instance name: " + cameraComponent->Name());
    }
    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mPlayer.reset();
}
