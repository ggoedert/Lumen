//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include <lArrayProperty.h>
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
    ARRAYPROPERTY(int, VTest, 3, GetVInt, SetVInt);
    PROPERTY(std::string, StrProp, GetStr, SetStr);

    ARRAYPROPERTY_RO(int, RVTest, 3, GetVInt);
    ARRAYPROPERTY_WO(int, WVTest, 3, SetVInt);

    [[nodiscard]] const int GetInt() const { return mInt; }
    void SetInt(const int &val) { mInt = val; }

    [[nodiscard]] int &GetVInt(size_t i) {
        return mVec[i];
    }
    void SetVInt(size_t i, const int &val) {
        mVec[i] = val;
    }

    [[nodiscard]] std::string &GetStr() { return mStr; }
    void SetStr(const std::string &val) { mStr = val; }

    int mInt = 0;
    std::vector<int> mVec = std::vector<int>(3);
    std::string mStr;

    ARRAYPROPERTY(int, A, 3, GetVInt, SetVInt);
    ARRAYPROPERTY(int, B, 3, GetVInt, SetVInt);
    ARRAYPROPERTY(int, C, 3, GetVInt, SetVInt);
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
    L_ASSERT_MSG(!std::is_const_v<T>, "Property returned should not be const");
    std::string_view propName = prop.Name();

    testPlayer.Test() = 20;
    testPlayer.Test().Set(a);
    int b = testPlayer.Test();

    testPlayer.VTest()[0] = 1;
    testPlayer.VTest()[1] = 2;
    testPlayer.VTest()[2] = 3;
    int va = testPlayer.RVTest()[1];
    testPlayer.WVTest()[0] = ++va;
    va = testPlayer.VTest()[2] = 10;
    size_t vsize = testPlayer.VTest().Size();

    testPlayer.A() = { 1, 2, 3 };
    testPlayer.B() = { 10, 20, 30 };
    testPlayer.C() = { 100, 200, 300 };

    std::vector<int> aVec = testPlayer.A();

    int i = 0, j = 1, k = 2;
    testPlayer.A()[i] = testPlayer.B()[j] = testPlayer.C()[k];

    bool isIntBase = testPlayer.Test().Type() == Lumen::PodType("int");
    std::string_view playerName = testPlayer.Test().Name();

    Lumen::IProperty &iProperty = static_cast<Lumen::IProperty &>(testPlayer.Test());
    bool isInt = iProperty.Type() == Lumen::PodType("int");
    bool isFloat = iProperty.Type() == Lumen::PodType("float");

    OtherPlayer otherPlayer;

    otherPlayer.Test() = 10;
    int c = otherPlayer.Test();
    int d = otherPlayer.TestR();
    otherPlayer.TestW() = testPlayer.Test() = 42;

    bool hasRead = otherPlayer.TestW().HasRead();
    bool hasWrite = otherPlayer.TestW().HasWrite();

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
        cameraLock->AddComponent(Lumen::Camera::Type(), Lumen::Camera::Params({ 0.4509f, 0.8431f, 1.f, 1.f }));
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
            sphereLock->AddComponent(Lumen::MeshFilter::Type(), Lumen::MeshFilter::Params({ static_pointer_cast<Lumen::Mesh>(spherePtr) }));
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
