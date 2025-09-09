//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include <lDebugLog.h>
#include <lPropertyArray.h>
#include <lTransform.h>
#include <lCamera.h>
#include <lMesh.h>
#include <lMeshFilter.h>
#include <lTexture.h>
#include <lMaterial.h>
#include <lShader.h>

class Player
{
public:
    Player() = default;
    ~Player() = default;

    PROPERTY(int, Test, GetInt, SetInt);
    PROPERTYARRAY(int, VTest, 3, GetVInt, SetVInt);
    PROPERTY(std::string, StrProp, GetStr, SetStr);

    PROPERTYARRAY_RO(int, RVTest, 3, GetVInt);
    PROPERTYARRAY_WO(int, WVTest, 3, SetVInt);

    [[nodiscard]] const int GetInt() const { return mInt; }
    void SetInt(const int &val) { mInt = val; }

    [[nodiscard]] int &GetVInt(size_t i)
    {
        return mVec[i];
    }
    void SetVInt(size_t i, const int &val)
    {
        mVec[i] = val;
    }

    [[nodiscard]] std::string &GetStr() { return mStr; }
    void SetStr(const std::string &val) { mStr = val; }

    int mInt = 0;
    std::vector<int> mVec = std::vector<int>(3);
    std::string mStr;

    PROPERTYARRAY(int, A, 3, GetVInt, SetVInt);
    PROPERTYARRAY(int, B, 3, GetVInt, SetVInt);
    PROPERTYARRAY(int, C, 3, GetVInt, SetVInt);
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
    otherPlayer.TestW() = testPlayer.Test() = otherPlayer.TestR();

    bool hasRead = otherPlayer.TestW().HasRead();
    bool hasWrite = otherPlayer.TestW().HasWrite();

    // assignment from lvalue
    testPlayer.Test() = a;

    // assignment from rvalue
    testPlayer.Test() = 3;

    // assignment from lvalue with return
    a = testPlayer.Test() = 4;

    // assignment from rvalue with return
    a = testPlayer.Test() = int(5);

    // assignment from moved rvalue
    b = 6;
    a = testPlayer.Test() = std::move(b);

    // direct conversion
    a = testPlayer.Test();

    // chained assignment
    testPlayer.Test() = a = 7;

    // comparisons
    bool isTest = false;
    isTest = testPlayer.Test() == 7;
    isTest = testPlayer.Test() != 7;
    isTest = testPlayer.Test() == 8;
    isTest = testPlayer.Test() != 8;
    if (testPlayer.Test() == 7) {}
    if (testPlayer.Test() != 8) {}

    // implicit read
    a = testPlayer.Test();

    // equality checks
    if (testPlayer.Test() == 123) {}
    if (testPlayer.Test() != 456) {}

    // simple assignment
    testPlayer.Test() = a;

    // rvalue assignment
    testPlayer.Test() = 11;

    // chained assignment
    testPlayer.Test() = a = 12;

    // assignment from moved value
    b = 13;
    testPlayer.Test() = std::move(b);

    int x = 15;
    testPlayer.Test() = std::move(x);       // write-only or read-write T&& overload
    x = testPlayer.Test() = std::move(x);   // read-write T&& with return

    x = 5;
    otherPlayer.TestW() = x;  // This hits the void operator=(const T&)

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
        // load sphere mesh
        Lumen::Expected<Lumen::ObjectPtr> meshExp = mApplication.Assets().Import(
            "Library/lumen default resources",
            Lumen::Mesh::Type(),
            "Sphere"
        );
        if (!meshExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load default sphere mesh resource, {}", meshExp.Error());
            return false;
        }

        // add sphere mesh filter component
        sphereLock->AddComponent(Lumen::MeshFilter::Type(), Lumen::MeshFilter::Params { static_pointer_cast<Lumen::Mesh>(meshExp.Value()) });

        // load default checker gray texture
        Lumen::Expected<Lumen::ObjectPtr> textureExp = mApplication.Assets().Import(
            "Resources/lumen_builtin_extra",
            Lumen::Texture::Type(),
            "Default-Checker-Gray"
        );
        if (!textureExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load default checker gray texture resource, {}", textureExp.Error());
            return false;
        }

        // load simple diffuse shader
        Lumen::Expected<Lumen::ObjectPtr> shaderExp = mApplication.Assets().Import(
            "Resources/lumen_builtin_extra",
            Lumen::Shader::Type(),
            "Simple/Diffuse"
        );
        if (!shaderExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load simple diffuse shader resource, {}", shaderExp.Error());
            return false;
        }

        // setup texture property in shader
        Lumen::ShaderPtr shader = static_pointer_cast<Lumen::Shader>(shaderExp.Value());
        shader->SetProperty("_MainTex", textureExp.Value());

        // add material component
        sphereLock->AddComponent(Lumen::Material::Type(), Lumen::Material::Params { shader });
    }

    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mSphere.reset();
}
