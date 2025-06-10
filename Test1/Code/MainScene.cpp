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
    Lumen::AddCamera(mCamera, {0.4509f, 0.8431f, 1.f, 1.f});

    Lumen::Math::Vector col = mApplication.GetBackgroundColor();

    Lumen::DebugLog::Info("static Camera Component name: " + Lumen::Camera::ComponentName());
    auto cameraComponent = mCamera.lock().get()->GetComponent(Lumen::Camera::ComponentType()).lock();
    if (cameraComponent)
    {
        Lumen::DebugLog::Info("polymorphic Component instance name: " + cameraComponent->ComponentName());
    }
    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mPlayer.reset();
}
