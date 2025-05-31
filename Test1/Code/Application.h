//==============================================================================================================================================================================
/// \file
/// \brief     client application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef APPLICATION_H
#define APPLICATION_H

#include "MainScene.h"

#include "lApplication.h"
#include "lSceneManager.h"

#include <memory>
#include <string>

/// Application class
class Application : public Lumen::Application
{
public:
    /// constructor
    Application(const std::string &name, const int version) : Lumen::Application() {}

    /// load our test scene
    bool Load() override
    {
        mMainScene = std::make_shared<MainScene>(*this);
        return mSceneManager.Load(mMainScene);
    }

    /// unload our test scene
    void Unload() override
    {
        mSceneManager.Unload();
    }

private:
    std::shared_ptr<MainScene> mMainScene;
};

#endif
