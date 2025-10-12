//==============================================================================================================================================================================
/// \file
/// \brief     client application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include <lExpected.h>
#include <lAssets.h>
#include <lMaterial.h>

#include "Test1.h"

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
public:
    /// creates a smart pointer version of the builtin extra
    static Lumen::AssetFactoryPtr MakePtr()
    {
        return Lumen::AssetFactoryPtr(new Test1Factory());
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
    explicit Test1Factory()
    {
        auto materialInfo = MaterialInfo::MakePtr();
        Lumen::Assets::RegisterAssetInfo(materialInfo->Type(), materialInfo->Name(), materialInfo);
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

    Lumen::Assets::RegisterFactory(Test1Factory::MakePtr(), 1.0f);

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
