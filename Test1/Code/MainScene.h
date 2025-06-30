//==============================================================================================================================================================================
/// \file
/// \brief     main scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lApplication.h"
#include "lGameObject.h"
#include "lScene.h"

CLASS_PTR_DEF(MainScene);

/// MainScene class
class MainScene : public Lumen::Scene
{
    CLASS_NO_DEFAULT_CTOR(MainScene);
    CLASS_NO_COPY_MOVE(MainScene);
    CLASS_PTR_MAKER(MainScene);

public:
    /// load scene
    bool Load() override;

    /// unload scene
    void Unload() override;

private:
    /// constructor
    explicit MainScene(const Lumen::Application &application) :
        Lumen::Scene(), mApplication(application), mCamera(Lumen::GameObject::MakePtr()), mSphere(Lumen::GameObject::MakePtr()) {}

    /// application reference
    const Lumen::Application &mApplication;

    /// components
    Lumen::GameObjectWeakPtr mCamera;
    Lumen::GameObjectWeakPtr mSphere;
};
