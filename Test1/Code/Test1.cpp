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
    [[nodiscard]] Lumen::HashType Type() const
    {
        return Lumen::Material::Type();
    }

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Material";
    }

    /// import the material
    [[nodiscard]] Lumen::Expected<Lumen::ObjectPtr> Import(Lumen::EngineWeakPtr &engine)
    {
        auto materialExpected = Lumen::Material::MakePtr("Simple/Diffuse");
        if (!materialExpected)
        {
            return Lumen::Expected<Lumen::ObjectPtr>::Unexpected(materialExpected.Error());
        }
        return Lumen::Expected<Lumen::ObjectPtr>(std::static_pointer_cast<Lumen::Object>(materialExpected.Value()));
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
    [[nodiscard]] std::span<const Lumen::AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const override
    {
        if (path == "Assets")
        {
            return mAssetInfos;
        }
        return {};
    }

private:
    /// constructor
    explicit Test1Factory(float priority) : Lumen::AssetFactory(priority)
    {
        auto materialInfo = MaterialInfo::MakePtr();
        Lumen::Assets::RegisterAssetInfo(materialInfo->Type(), materialInfo->Name(), materialInfo, priority);
        mAssetInfos.push_back(materialInfo);
    }

    /// asset infos
    std::vector<Lumen::AssetInfoPtr> mAssetInfos;
};

/// initialize and load our test scene
bool Test1::Initialize()
{
    if (!Lumen::Application::Initialize())
    {
        return false;
    }

    Lumen::Assets::RegisterFactory(Test1Factory::MakePtr(2.0f));

    if (auto engineLock = GetEngine().lock())
    {
        Lumen::FileSystem::RegisterFileSystem(engineLock->FolderFileSystem("Assets", "Assets"));
    }

    mMainScene = MainScene::MakePtr(*this);
    bool loadResult = Lumen::SceneManager::Load(mMainScene);
    if (!loadResult)
    {
        Shutdown();
    }
    return loadResult;
}

/// shutdown and unload our test scene
void Test1::Shutdown()
{
    Lumen::SceneManager::Unload();
    Lumen::Application::Shutdown();
}
