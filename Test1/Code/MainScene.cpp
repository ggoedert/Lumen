//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include <lTransform.h>
#include <lCamera.h>
#include <lMesh.h>
#include <lMeshFilter.h>

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
    testPlayer.Test() = 10;
    int a = testPlayer.Test().Get();
    testPlayer.StrProp() = "my string prop";

    auto &prop = testPlayer.Test();
    using T = decltype(prop);  // What is T?
    LUMEN_ASSERT(!std::is_const_v<T> && "Property returned should not be const");
    std::string_view propName = prop.Name();

    testPlayer.Test() = 20;
    testPlayer.Test().Set(a);
    int b = testPlayer.Test();

    std::vector<int> v = testPlayer.VTest();
    v[1] = 10;
    v[8] = 100;
    testPlayer.VTest() = v;
    bool isIntBase = testPlayer.Test().IsType(Lumen::NameType("int"));
    std::string_view playerName = testPlayer.Test().Name();

    Lumen::IProperty &iProperty = static_cast<Lumen::IProperty &>(testPlayer.Test());
    bool isInt = iProperty.IsType(Lumen::NameType("int"));
    bool isFloat = iProperty.IsType(Lumen::NameType("float"));

    OtherPlayer otherPlayer;

    otherPlayer.Test() = 10;
    int c = otherPlayer.Test();
    int d = otherPlayer.TestR();
    otherPlayer.TestW() = testPlayer.Test() = 42;

    // Uncommenting below lines will give compile-time errors with messages:
    //int z = otherPlayer.TestW();   // error: read from write-only
    //otherPlayer.TestR() = 42;      // error: write to read-only

    // setup camera
    if (auto cameraLock = mCamera.lock())
    {
        if (auto transformLock = cameraLock->Transform().lock())
        {
            transformLock->Position() = { 0.f, 0.f, -10.f };
        }

        WARNING_DISABLE_NODISCARD_PUSH();
        cameraLock->AddComponent(Lumen::Camera::Type(), Lumen::Camera::Params({ 0.4509f, 0.8431f, 1.f, 1.f }));
        WARNING_POP();
    }

    // setup sphere
    if (auto sphereLock = mSphere.lock())
    {
        Lumen::ObjectPtr spherePtr = mApplication.Resources().Import(
            "Library/lumen default resources",
            Lumen::Mesh::Type(),
            "Sphere"
        );
        if (spherePtr && spherePtr->Type() == Lumen::Mesh::Type())
        {
            WARNING_DISABLE_NODISCARD_PUSH();
            sphereLock->AddComponent(Lumen::MeshFilter::Type(), Lumen::MeshFilter::Params({ static_pointer_cast<Lumen::Mesh>(spherePtr) }));
            WARNING_POP();
        }
    }

    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mSphere.reset();
}
