//==============================================================================================================================================================================
/// \file
/// \brief     client application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "MainScene.h"

#include "lApplication.h"
#include "lSceneManager.h"
#include "lFileSystem.h"
#include "lEngine.h"

CLASS_PTR_DEF(Test1);

/// Test1 class
class Test1 : public Lumen::Application
{
    CLASS_NO_DEFAULT_CTOR(Test1);
    CLASS_NO_COPY_MOVE(Test1);
    CLASS_PTR_MAKER(Test1);

public:
    /// constructor
    explicit Test1(const std::string &name, const int version) : Lumen::Application() {}

    /// initialize and load our test scene
    bool Initialize() override
    {
        if (auto engineLock = GetEngine().lock())
        {
            Lumen::FileSystem::RegisterFileSystem(engineLock->FolderFileSystem("Assets", "Assets"));
        }
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
