//==============================================================================================================================================================================
/// \file
/// \brief     client application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lApplication.h"
#include "lScene.h"

CLASS_PTR_DEF(Test1);

/// Test1 class
class Test1 : public Lumen::Application
{
    CLASS_NO_DEFAULT_CTOR(Test1);
    CLASS_NO_COPY_MOVE(Test1);
    CLASS_PTR_MAKER(Test1);

public:
    /// initialize test1
    void Initialize() override;

    /// shutdown test1
    void Shutdown() override;

    /// TEMP DELME!
    void New() override;

    /// TEMP DELME!
    void Open() override;

private:
    /// constructor
    explicit Test1(std::string_view name, const int version) : Lumen::Application() {}

    /// main scene
    Lumen::ScenePtr mMainScene;
};
