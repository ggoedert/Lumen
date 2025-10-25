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
    GameObjectWeakPtr &CreateGameObject() { return mGameObjects.emplace_back(Lumen::GameObject::MakePtr(mApplication)); }

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

    /* NEW PORTING */

    const std::string mainScene = R"(
        [
            {
                "Name": "Camera",
                "Transform": { "Position": [ 0, 0, -10 ] },
                "Components": { "Lumen::Camera": { "Params": { "BackgroundColor": [ 0.4509, 0.8431, 1, 1 ] } } }
            },
            {
                "Name": "Sphere",
                "Components": {
                    "Lumen::MeshFilter": { "Params": { "Lumen::Mesh": { "Path": "Library/lumen default resources", "Name": "Sphere" } } },
                    "Lumen::MeshRenderer": {
                        "Params": { "Lumen::Material": { "Path": "Assets", "Name": "Material" } },
                        "Properties": { "_MainTex": { "Lumen::Texture": { "Path": "Resources/lumen_builtin_extra", "Name": "Default-Checker-Gray" } } }
                    },
                    "SphereScript": {}
                }
            }
        ]
        )";

    // parse json
    json parsed = json::parse(mainScene);

    // You can optionally inspect/access data here:
    for (auto &obj : parsed)
    {
/*
        Lumen::DebugLog::Info("Object name: {}", obj["Name"].get<std::string>());
        if (obj.contains("Transform"))
            Lumen::DebugLog::Info("Has Transform");
        if (obj.contains("Components"))
            Lumen::DebugLog::Info("Has {} components", obj["Components"].size());
*/
        if (obj.contains("Name")) // game objects must have a name
        {
            if (auto gameObjectLock = mImpl->CreateGameObject().lock())
            {
                gameObjectLock->Deserialize(obj);
/*
                if (auto transformLock = gameObjectLock->Transform().lock())
                {
                    transformLock->SetPosition({ 0.f, 0.f, -10.f });
                }
                gameObjectLock->AddComponent(
                    Lumen::Camera::Type(),
                    Lumen::Camera::Params({ 0.4509f, 0.8431f, 1.f, 1.f }));
*/
            }
        }
    }

#ifdef OLD_PORTING
    // setup camera
    if (auto cameraLock = mImpl->CreateGameObject().lock())
    {
        if (auto transformLock = cameraLock->Transform().lock())
        {
            transformLock->SetPosition({ 0.f, 0.f, -10.f });
        }
        cameraLock->AddComponent(
            Lumen::Camera::Type(),
            Lumen::Camera::Params({ 0.4509f, 0.8431f, 1.f, 1.f }));
    }

    // setup sphere
    if (auto sphereLock = mImpl->CreateGameObject().lock())
    {
        // load sphere mesh
        Lumen::Expected<Lumen::ObjectPtr> meshExp = Lumen::Assets::Import(
            "Library/lumen default resources",
            Lumen::Mesh::Type(),
            "Sphere"
        );
        if (!meshExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load default sphere mesh resource, {}", meshExp.Error());
            return false;
        }

        // add sphere mesh filter component, with mesh
        sphereLock->AddComponent(
            Lumen::MeshFilter::Type(),
            Lumen::MeshFilter::Params { static_pointer_cast<Lumen::Mesh>(meshExp.Value()) });

        // load default checker gray texture
        Lumen::Expected<Lumen::ObjectPtr> textureExp = Lumen::Assets::Import(
            "Resources/lumen_builtin_extra",
            Lumen::Texture::Type(),
            "Default-Checker-Gray"
        );
        if (!textureExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load default checker gray texture resource, {}", textureExp.Error());
            return false;
        }
        const Lumen::TexturePtr texture = static_pointer_cast<Lumen::Texture>(textureExp.Value());

        // load material material
        Lumen::Expected<Lumen::ObjectPtr> materialExp = Lumen::Assets::Import(
            "Assets",
            Lumen::Material::Type(),
            "Material"
        );
        if (!materialExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load material resource, {}", materialExp.Error());
            return false;
        }

        // add mesh renderer component, with material
        if (auto meshRendererLock = static_pointer_cast<Lumen::MeshRenderer>(sphereLock->AddComponent(
            Lumen::MeshRenderer::Type(),
            Lumen::MeshRenderer::Params { static_pointer_cast<Lumen::Material>(materialExp.Value()) }).lock()))
        {
            // set texture property
            meshRendererLock->SetProperty("_MainTex", texture);
        }

        // add sphere behaviour script
        sphereLock->AddComponent(
            Lumen::EncodeType("SphereScript"),
            Behavior::Params {});
    }
#endif

    return true;
}

/// unload scene
void Scene::Unload()
{
    Lumen::DebugLog::Info("Scene::Unload");
    mImpl->ReleaseGameObjects();
}
