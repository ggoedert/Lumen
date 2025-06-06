//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include "lDebugLog.h"
#include "lCamera.h"
#include "lApplication.h"

bool MainScene::Load()
{
    Lumen::DebugLog::Info("MainScene::Load");

    mPlayer = Lumen::GameObject::MakePtr(mApplication.GetSceneManager());

    mCamera = Lumen::GameObject::MakePtr(mApplication.GetSceneManager());
    Lumen::AddCamera(mCamera, {0.4509f, 0.8431f, 1.f, 1.f});

    Lumen::Math::Vector col = mCamera->GetApplication().GetBackgroundColor();

    Lumen::DebugLog::Info("static Camera Component name: " + Lumen::Camera::ComponentName());
    Lumen::DebugLog::Info("polymorphic Component instance name: " + mCamera->GetComponent(Lumen::Camera::ComponentType())->ComponentName());
    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mPlayer.reset();
}
