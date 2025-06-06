//==============================================================================================================================================================================
/// \file
/// \brief     client application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "MainScene.h"

#include "lApplication.h"

#include <memory>
#include <string>

CLASS_PTR_DEFS(Application);

/// Application class
class Application : public Lumen::Application
{
    CLASS_PTR_MAKERS(Application);

public:
    /// constructor
    Application(const std::string &name, const int version) : Lumen::Application() {}

    /// load our test scene
    bool Load() override
    {
        mMainScene = MainScene::MakePtr(*this);
        return GetSceneManager().Load(mMainScene);
    }

    /// unload our test scene
    void Unload() override
    {
        GetSceneManager().Unload();
    }

private:
    MainScenePtr mMainScene;
};
