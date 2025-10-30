//==============================================================================================================================================================================
/// \file
/// \brief     Scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lScene.h"
#include "lSceneManager.h"

using namespace Lumen;

/// Scene::Impl class
class Scene::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs a behavior
    explicit Impl(Lumen::Application &application) : mApplication(application) {}

    /// destroys behavior
    ~Impl() = default;

public:
    /// serialize
    void Serialize(SerializedData &out, bool packed) const
    {
    }

    /// deserialize
    void Deserialize(const SerializedData &in, bool packed)
    {
        for (auto &gameObject : in.items())
        {
            if (auto gameObjectLock = mGameObjects.emplace_back(Lumen::GameObject::MakePtr(mApplication, gameObject.key())).lock())
            {
                gameObjectLock->Deserialize(gameObject.value(), packed);
            }
        }
    }

    /// load scene
    bool Load(const std::filesystem::path &file)
    {
        Lumen::DebugLog::Info("Scene::Load {}", file.string());

        const std::string mainScene = R"(
        {
            "Camera": {
                "Transform": { "Position": [ 0, 0, -10 ] },
                "Components": { "Lumen::Camera": { "BackgroundColor": [ 0.4509, 0.8431, 1, 1 ] } }
            },
            "Sphere" : {
                "Components": {
                    "Lumen::MeshFilter": { "Lumen::Mesh": { "Path": "lumen default resources/Assets/Mesh/Sphere.fbx", "Name": "Sphere" } },
                    "Lumen::MeshRenderer": {
                        "Lumen::Material": { "Path": "Assets/Material.mat", "Name": "Material" },
                        "Properties": { "_MainTex": { "Lumen::Texture": { "Path": "lumen_builtin_extra/Assets/Texture2D/Default-Checker-Gray.png", "Name": "Default-Checker-Gray" } } }
                    },
                    "SphereScript": {}
                }
            }
        }
        )";

        // parse the scene
        try
        {
            const SerializedData in = SerializedData::parse(mainScene);
            bool packed = in.contains("$") && in["$"].get<int>() == 42;
            Deserialize(in, packed);
        }
        catch (const SerializedData::exception &e)
        {
            Lumen::DebugLog::Error("{}", e.what());
        }
        catch (const std::exception &e)
        {
            Lumen::DebugLog::Error("Deserialization error, {}", e.what());
        }

        return true;
    }

    /// unload scene
    void Unload()
    {
        Lumen::DebugLog::Info("Scene::Unload");
        for (auto gameObjectWeakPtr : mGameObjects)
        {
            SceneManager::UnregisterGameObject(gameObjectWeakPtr);
        }
        mGameObjects.clear();
    }

private:
    /// application reference
    Lumen::Application &mApplication;

    /// game objects in the scene
    GameObjects mGameObjects;
};

//==============================================================================================================================================================================

/// constructor
Scene::Scene(Lumen::Application &application) : mImpl(Scene::Impl::MakeUniquePtr(application)) {}

/// destructor
Scene::~Scene() = default;

/// serialize
void Scene::Serialize(SerializedData &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Scene::Deserialize(const SerializedData &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// load scene
bool Scene::Load(const std::filesystem::path &file)
{
    return mImpl->Load(file);
}

/// unload scene
void Scene::Unload()
{
    return mImpl->Unload();
}
