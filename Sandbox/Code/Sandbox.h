//==============================================================================================================================================================================
/// \file
/// \brief     client application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lApplication.h"
#include "lScene.h"

CLASS_PTR_DEF(Sandbox);

/// Sandbox class
class Sandbox : public Lumen::Application
{
    CLASS_NO_DEFAULT_CTOR(Sandbox);
    CLASS_NO_COPY_MOVE(Sandbox);
    CLASS_PTR_MAKER(Sandbox);

public:
    /// initialize sandbox
    void Initialize(const Lumen::ApplicationWeakPtr &application) override;

    /// shutdown sandbox
    void Shutdown() override;

    /// get winwdow size
    void GetWindowSize(int &width, int &height) override;

    /// TEMP DELME!
    void New() override;

    /// TEMP DELME!
    void Open() override;

private:
    /// constructor
    explicit Sandbox(std::string_view name, const int version) : Lumen::Application() {}

    /// main scene
    Lumen::ScenePtr mMainScene;
};
