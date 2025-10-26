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
    /// create new game object on the scene
    GameObjectWeakPtr &CreateGameObject(std::string_view name) { return mGameObjects.emplace_back(Lumen::GameObject::MakePtr(mApplication, name)); }

    /// release game objects
    void ReleaseGameObjects()
    {
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

/// load scene
bool Scene::Load(const std::filesystem::path &file)
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
                    "Lumen::MeshFilter": { "Lumen::Mesh": { "Path": "Library/lumen default resources", "Name": "Sphere" } },
                    "Lumen::MeshRenderer": {
                        "Lumen::Material": { "Path": "Assets", "Name": "Material" },
                        "Properties": { "_MainTex": { "Lumen::Texture": { "Path": "Resources/lumen_builtin_extra", "Name": "Default-Checker-Gray" } } }
                    },
                    "SphereScript": {}
                }
            }
        }
        )";

    // parse the scene
    try
    {
        json parsed = json::parse(mainScene);
        for (auto &gameObject : parsed.items())
        {
            if (auto gameObjectLock = mImpl->CreateGameObject(gameObject.key()).lock())
            {
                gameObjectLock->Deserialize(gameObject.value());
            }
        }
    }
    catch (const json::exception &e)
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
void Scene::Unload()
{
    Lumen::DebugLog::Info("Scene::Unload");
    mImpl->ReleaseGameObjects();
}
