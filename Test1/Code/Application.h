//==============================================================================================================================================================================
/// \file
/// \brief     client application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "MainScene.h"

#include "lApplication.h"
#include "lSceneManager.h"

#include <memory>
#include <string>

CLASS_PTR_DEF(Application);

/// Application class
class Application : public Lumen::Application
{
    CLASS_NO_DEFAULT_CTOR(Application);
    CLASS_NO_COPY_MOVE(Application);
    CLASS_PTR_MAKER(Application);

public:
    /// constructor
    Application(const std::string &name, const int version) : Lumen::Application() {}

    /// load our test scene
    bool Load() override
    {
        mMainScene = MainScene::MakePtr(*this);
        return Lumen::SceneManager::Load(mMainScene);
    }

    /// unload our test scene
    void Unload() override
    {
        Lumen::SceneManager::Unload();
    }

private:
    MainScenePtr mMainScene;
};
