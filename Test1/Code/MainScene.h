//==============================================================================================================================================================================
/// \file
/// \brief     main scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include "lScene.h"

/// MainScene class
class MainScene : public Lumen::Scene
{
public:
    /// constructor
    MainScene(Lumen::Application &application) : Scene(application) {}

    /// load scene
    bool Load() override;

    /// unload scene
    void Unload() override;
};

#endif
