//==============================================================================================================================================================================
/// \file
/// \brief     client application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "Sandbox.h"

#include "lMaterial.h"
#include "lAssetManager.h"
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
        if (!materialExpected.Value()->Load())
        {
            return Lumen::Expected<Lumen::AssetPtr>::Unexpected(std::format("Unable to load material resource, {}", Path()));
        }
        return materialExpected;
    }

private:
    /// constructs a material info
    explicit MaterialInfo() {}
};

/// SandboxFactory class
class SandboxFactory : public Lumen::AssetFactory
{
    CLASS_NO_DEFAULT_CTOR(SandboxFactory);

public:
    /// creates a smart pointer version of the builtin extra
    static Lumen::AssetFactoryPtr MakePtr(float priority)
    {
        return Lumen::AssetFactoryPtr(new SandboxFactory(priority));
    }

    /// import asset
    [[nodiscard]] Lumen::AssetPtr Import(Lumen::EngineWeakPtr &engine, Lumen::HashType type, const std::filesystem::path &path) const override
    {
        for (auto assetInfo : mAssetInfos)
        {
            if ((assetInfo->Type() == type) && (assetInfo->Path() == path))
            {
                auto assetExpected = assetInfo->Import(engine);
                if (assetExpected)
                {
                    return assetExpected.Value();
                }
                else
                {
                    Lumen::DebugLog::Error("SandboxFactory import, {}", assetExpected.Error());
                }
            }
        }
        return nullptr;
    }

private:
    /// constructor
    explicit SandboxFactory(float priority) : Lumen::AssetFactory(priority)
    {
        mAssetInfos.push_back(MaterialInfo::MakePtr());
    }

    /// asset infos
    std::vector<Lumen::AssetInfoPtr> mAssetInfos;
};

/// initialize sandbox
void Sandbox::Initialize()
{
    Lumen::Application::Initialize();

    Lumen::AssetManager::RegisterFactory(SandboxFactory::MakePtr(2.0f));

    if (auto engineLock = GetEngine().lock())
    {
        Lumen::FileSystem::RegisterFileSystem("Assets", engineLock->AssetsFileSystem());
    }
}

/// shutdown sandbox
void Sandbox::Shutdown()
{
    /// TEMP DELME!
    if (auto engineLock = GetEngine().lock())
    {
        engineLock->New();
    }
}

/// TEMP DELME!
void Sandbox::New()
{
    mMainScene.reset();
}

/// TEMP DELME!
void Sandbox::Open()
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
