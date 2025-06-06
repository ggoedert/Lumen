//==============================================================================================================================================================================
/// \file
/// \brief     main scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lGameObject.h"
#include "lScene.h"

CLASS_PTR_DEFS(MainScene);

/// MainScene class
class MainScene : public Lumen::Scene
{
    CLASS_PTR_MAKERS(MainScene);

public:
    /// constructor
    MainScene(Lumen::Application &application) : Scene(application) {}

    /// load scene
    bool Load() override;

    /// unload scene
    void Unload() override;

private:
    Lumen::GameObjectPtr mPlayer;
    Lumen::GameObjectPtr mCamera;
};
