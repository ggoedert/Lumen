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
    explicit Application(const std::string &name, const int version) : Lumen::Application() {}

    /// initialize and load our test scene
    bool Initialize() override
    {
        mMainScene = MainScene::MakePtr(*this);
        return Lumen::SceneManager::Load(mMainScene);
    }

    /// shutdown and unload our test scene
    void Shutdown() override
    {
        Lumen::SceneManager::Unload();
    }

private:
    MainScenePtr mMainScene;
};
