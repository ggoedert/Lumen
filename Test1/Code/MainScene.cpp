//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include "lCamera.h"

#include <lProperty.h>

class Player
{
public:
    Player() = default;
    ~Player() = default;

    PROPERTY(int, Test, GetInt, SetInt);
    PROPERTY(std::vector<int>, VTest, GetVInt, SetVInt);
    PROPERTY(std::string, StrProp, GetStr, SetStr);

    [[nodiscard]] const int GetInt() const { return mInt; }
    void SetInt(const int &val) { mInt = val; }

    [[nodiscard]] std::vector<int> &GetVInt() { return mVec; }
    void SetVInt(const std::vector<int> &val) { mVec = val; }

    [[nodiscard]] std::string &GetStr() { return mStr; }
    void SetStr(const std::string &val) { mStr = val; }

    int mInt = 0;
    std::vector<int> mVec = std::vector<int>(10);
    std::string mStr;
};

class OtherPlayer
{
public:
    OtherPlayer() = default;
    ~OtherPlayer() = default;

    PROPERTY(int, Test, GetInt, SetInt);
    PROPERTY_RO(int, TestR, GetInt);
    PROPERTY_WO(int, TestW, SetInt);

    [[nodiscard]] const int GetInt() const { return 10/2; }
    void SetInt(const int &val) { mInt = val; }

private:
    int mInt = 0;
};

bool MainScene::Load()
{
    Lumen::DebugLog::Info("MainScene::Load");

    Player testPlayer;
    testPlayer.Test().Set(10);
    std::any a = testPlayer.Test().Get();

    auto &prop = testPlayer.Test();
    using T = decltype(prop);  // What is T?
    static_assert(!std::is_const_v<T>, "Property returned should not be const");

    testPlayer.Test() = 20;
    testPlayer.Test().Set(a);
    int b = testPlayer.Test();
    testPlayer.VTest()[1] = 10;
    testPlayer.VTest()[8] = 100;
    bool isIntBase = testPlayer.Test().IsTypeId(typeid(int));
    std::string_view name = testPlayer.Test().Name();

    Lumen::IProperty &iProperty = static_cast<Lumen::IProperty &>(testPlayer.Test());
    bool isInt = iProperty.IsTypeId(typeid(int));
    bool isFloat = iProperty.IsTypeId(typeid(float));

    OtherPlayer otherPlayer;

    otherPlayer.Test() = 10;
    int c = otherPlayer.Test();      // ok
    c = otherPlayer.Test()++;        // increment
    int d = otherPlayer.TestR();     // ok
    otherPlayer.TestW() = 20;        // ok

    // Uncommenting below lines will give compile-time errors with messages:
    //int z = otherPlayer.TestW();   // error: read from write-only
    //otherPlayer.TestR() = 42;      // error: write to read-only

    if (auto gameObjectLock = mCamera.lock())
    {
        gameObjectLock->AddComponent(Lumen::Camera::Type(), Lumen::Camera::Params({ {0.4509f, 0.8431f, 1.f, 1.f} }));

        Lumen::Math::Vector col = mApplication.BackgroundColor();

        Lumen::DebugLog::Info("Static component name: " + Lumen::Camera::Name());
        if (auto cameraComponentLock = gameObjectLock->Component(Lumen::Camera::Type()).lock())
        {
            Lumen::DebugLog::Info("Polymorphic component instance name: " + cameraComponentLock->Name());
        }
    }
    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mPlayer.reset();
}
