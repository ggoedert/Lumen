//==============================================================================================================================================================================
/// \file
/// \brief     client application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef APPLICATION_H
#define APPLICATION_H

#include "MainScene.h"

#include "lApplication.h"

#include <memory>
#include <string>

/// Application class
class Application : public Lumen::Application
{
    CLASS_UTILS(Application);

public:
    /// constructor
    Application(const std::string &name, const int version) : Lumen::Application() {}

    /// load our test scene
    bool Load() override
    {
        mMainScene = MainScene::MakePtr(*this);
        return mSceneManager.Load(mMainScene);
    }

    /// unload our test scene
    void Unload() override
    {
        mSceneManager.Unload();
    }

private:
    MainScene::Ptr mMainScene;
};

#endif
