//==============================================================================================================================================================================
/// \file
/// \brief     client application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "Test1.h"
#include "SphereScript.h"

#include "lMaterial.h"
#include "lSceneManager.h"
#include "lEngine.h"

#include <fstream>

/// MaterialInfo class
class MaterialInfo : public Lumen::AssetInfo
{
public:
    /// creates a smart pointer version of the material info
    static Lumen::AssetInfoPtr MakePtr()
    {
        return Lumen::AssetInfoPtr(new MaterialInfo());
    }

    /// get type
    [[nodiscard]] Lumen::HashType Type() const override
    {
        return Lumen::Material::Type();
    }

    /// get path
    [[nodiscard]] std::string_view Path() const override
    {
        return "Assets/Material.mat";
    }

    /// import the material
    [[nodiscard]] Lumen::Expected<Lumen::AssetPtr> Import(Lumen::EngineWeakPtr &engine)
    {
        auto materialExpected = Lumen::Material::MakePtr(Path());
        if (!materialExpected)
        {
            return Lumen::Expected<Lumen::AssetPtr>::Unexpected(materialExpected.Error());
        }
        return Lumen::Expected<Lumen::AssetPtr>(materialExpected.Value());
    }

private:
    /// constructs a material info
    explicit MaterialInfo() {}
};

/// Test1Factory class
class Test1Factory : public Lumen::AssetFactory
{
    CLASS_NO_DEFAULT_CTOR(Test1Factory);

public:
    /// creates a smart pointer version of the builtin extra
    static Lumen::AssetFactoryPtr MakePtr(float priority)
    {
        return Lumen::AssetFactoryPtr(new Test1Factory(priority));
    }

    /// get asset infos
    [[nodiscard]] std::vector<Lumen::AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const override
    {
        std::vector<Lumen::AssetInfoPtr> result;
        for (auto assetInfo : mAssetInfos)
        {
            if (assetInfo->Path() == path)
            {
                result.push_back(assetInfo);
            }
        }
        return result;
    }

private:
    /// constructor
    explicit Test1Factory(float priority) : Lumen::AssetFactory(priority)
    {
        mAssetInfos.push_back(MaterialInfo::MakePtr());
    }

    /// asset infos
    std::vector<Lumen::AssetInfoPtr> mAssetInfos;
};

/// initialize test1
void Test1::Initialize()
{
    Lumen::Application::Initialize();

    Lumen::AssetManager::RegisterFactory(Test1Factory::MakePtr(2.0f));

    if (auto engineLock = GetEngine().lock())
    {
        Lumen::FileSystem::RegisterFileSystem("Assets", engineLock->AssetsFileSystem());
    }
}

/// shutdown test1
void Test1::Shutdown()
{
    /// TEMP DELME!
    if (auto engineLock = GetEngine().lock())
    {
        engineLock->New();
    }
}

/// TEMP DELME!
void Test1::New()
{
    mMainScene.reset();
}

/// TEMP DELME!
void Test1::Open()
{
    if (auto engineLock = GetEngine().lock())
    {
        engineLock->New();
    }
    mMainScene = Lumen::Scene::MakePtr(*this, "Assets/MainScene.lumen");
    if (!Lumen::SceneManager::Load(mMainScene))
    {
        Shutdown();
    }
    else
    {
        Lumen::Serialized::Type out;
        //out["data"] = Serialized::Type::binary({ 0x01, 0x02, 0x03, 99, 100, 101, 127, 128, 129, 254, 255 });
        mMainScene->Serialize(out, false);
        std::ofstream("Assets\\serializer_test.txt") << out.dump(4);
    }

}
