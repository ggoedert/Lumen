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
            if (auto gameObjectLock = mGameObjects.emplace_back(Lumen::GameObject::MakePtr(mApplication, gameObject.key())).lock())
            {
                gameObjectLock->Deserialize(gameObject.value(), packed);
            }
        }
    }

    /// save scene
    bool Save() const
    {
        /*
        Lumen::DebugLog::Info("Scene::Save {}", outFile.string());
        Serialized::Type out;
        Serialize(out, false);
        std::ofstream file(outFile);
        if (!file.is_open())
        {
            Lumen::DebugLog::Error("Unable to open scene file for writing, {}", outFile.string());
            return false;
        }
        file << out.dump(4);
        file.close();
        return true;
        */
        return true;
    }

    /// load scene
    bool Load()
    {
        const std::filesystem::path &path = mOwner.lock()->Path();
        Lumen::DebugLog::Info("Scene::Load {}", path.string());

        // read the scene
        auto [mainScene, packed] = FileSystem::ReadSerializedData(path);
        //packed = true; //@REVIEW@ testing!
        if (mainScene.empty())
        {
            Lumen::DebugLog::Error("Unable to read the scene");
            return false;
        }

        // parse the scene
        try
        {
            const Serialized::Type in = Serialized::Type::parse(mainScene);
            Deserialize(in, packed);
        }
        catch (const std::exception &e)
        {
            Lumen::DebugLog::Error("{}", e.what());
            return false;
        }

/*
        Serialized::Type out;
        //out["data"] = Serialized::Type::binary({ 0x01, 0x02, 0x03, 99, 100, 101, 127, 128, 129, 254, 255 });
        Serialize(out, true);
        std::ofstream("Assets\\serializer_test.txt") << out.dump(4);
*/

        return true;
    }

    /// release scene
    void Release()
    {
        if (!mGameObjects.empty())
        {
            Lumen::DebugLog::Info("Scene::Release");
            for (auto gameObjectWeakPtr : mGameObjects)
            {
                SceneManager::UnregisterGameObject(gameObjectWeakPtr);

            }
            mGameObjects.clear();
        }
    }

    /// owner
    SceneWeakPtr mOwner;

    /// application reference
    Lumen::Application &mApplication;

    /// game objects in the scene
    GameObjects mGameObjects;
};

//==============================================================================================================================================================================

/// constructor
Scene::Scene(Lumen::Application &application, std::string_view name, const std::filesystem::path &path) :
    Asset(Type(), name, path), mImpl(Scene::Impl::MakeUniquePtr(application)) {}

/// destroys scene
Scene::~Scene()
{
    Release();
}

/// creates a smart pointer version of the scene
ScenePtr Scene::MakePtr(Lumen::Application &application, std::string_view name, const std::filesystem::path &path)
{
    auto ptr = ScenePtr(new Scene(application, name, path));
    ptr->mImpl->mOwner = ptr;
    return ptr;
}

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

/// save scene
bool Scene::Save() const
{
    return mImpl->Save();
}

/// load scene
bool Scene::Load()
{
    return mImpl->Load();
}

/// release scene
void Scene::Release() 
{
    mImpl->Release();
}
