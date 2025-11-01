//==============================================================================================================================================================================
/// \file
/// \brief     Scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lScene.h"
#include "lSceneManager.h"

#include <fstream>

using namespace Lumen;

/// Scene::Impl class
class Scene::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs a behavior
    explicit Impl(Lumen::Application &application) : mApplication(application) {}

    /// destroys behavior
    ~Impl() = default;

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        if (packed)
        {
            out["$"] = Serialized::Type::object();
        }
        for (auto &gameObject : mGameObjects)
        {
            if (auto gameObjectLock = gameObject.lock())
            {
                Serialized::Type gameObjectData;
                gameObjectLock->Serialize(gameObjectData, packed);
                out[gameObjectLock->Name()] = gameObjectData;
            }
        }

        // if empty, set to object
        if (out.empty())
        {
            out = Serialized::Type::object();
        }
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        for (auto &gameObject : in.items())
        {
            if (packed && gameObject.key() == "$")
            {
                continue;
            }
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

        const std::string mainScene =
#ifndef PACKED
R"(
{
    "Camera": {
        "Transform": {
            "Position": [
                0.0,
                0.0,
                -10.0
            ]
        },
        "Components": {
            "Lumen::Camera": {
                "BackgroundColor": [
                    0.45089998841285706,
                    0.8431000113487244,
                    1.0,
                    1.0
                ]
            }
        }
    },
    "Sphere": {
        "Components": {
            "Lumen::MeshFilter": {
                "Lumen::Mesh": {
                    "Path": "lumen default resources/Assets/Mesh/Sphere.fbx",
                    "Name": "Sphere"
                }
            },
            "Lumen::MeshRenderer": {
                "Lumen::Material": {
                    "Path": "Assets/Material.mat",
                    "Name": "Material"
                },
                "Properties": {
                    "_MainTex": {
                        "Lumen::Texture": {
                            "Path": "lumen_builtin_extra/Assets/Texture2D/Default-Checker-Gray.png",
                            "Name": "Default-Checker-Gray"
                        }
                    }
                }
            },
            "SphereScript": {}
        }
    }
}
)";
#else
R"(
{
    "$": {},
    "Camera": {
        "rYKruw": {
            "4n80Kg": [
                0.0,
                0.0,
                -10.0
            ]
        },
        "XGwDiQ": {
            "9sdi3Q": {
                "0UltMA": [
                    0.45089998841285706,
                    0.8431000113487244,
                    1.0,
                    1.0
                ]
            }
        }
    },
    "Sphere": {
        "XGwDiQ": {
            "XIWjMQ": {
                "WQDY9w": {
                    "Path": "lumen default resources/Assets/Mesh/Sphere.fbx",
                    "Name": "Sphere"
                }
            },
            "SJzguA": {
                "kzeNMw": {
                    "Path": "Assets/Material.mat",
                    "Name": "Material"
                },
                "gcgJ/A": {
                    "_MainTex": {
                        "NFQLMQ": {
                            "Path": "lumen_builtin_extra/Assets/Texture2D/Default-Checker-Gray.png",
                            "Name": "Default-Checker-Gray"
                        }
                    }
                }
            },
            "Di0Taw": {}
        }
    }
}
)";
#endif

        // parse the scene
        try
        {
            const Serialized::Type in = Serialized::Type::parse(mainScene);
            bool packed = in.contains("$") && in["$"].empty();
            Deserialize(in, packed);
        }
        catch (const Serialized::Type::exception &e)
        {
            Lumen::DebugLog::Error("{}", e.what());
        }
        catch (const std::exception &e)
        {
            Lumen::DebugLog::Error("Deserialization error, {}", e.what());
        }

        Serialized::Type out;
        Serialize(out, true);
        std::ofstream("serializer_test.txt") << out.dump(4);

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
void Scene::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Scene::Deserialize(const Serialized::Type &in, bool packed)
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
