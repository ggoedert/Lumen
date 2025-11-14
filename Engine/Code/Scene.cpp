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
        for (auto &entity : mEntities)
        {
            if (auto entityLock = entity.lock())
            {
                Serialized::Type entityData;
                entityLock->Serialize(entityData, packed);
                out[entityLock->Name()] = entityData;
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
        for (auto &entity : in.items())
        {
            if (auto entityLock = mEntities.emplace_back(Lumen::Entity::MakePtr(mApplication, entity.key())).lock())
            {
                entityLock->Deserialize(entity.value(), packed);
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

        return true;
    }

    /// release scene
    void Release()
    {
        if (!mEntities.empty())
        {
            Lumen::DebugLog::Info("Scene::Release");
            for (auto entityWeakPtr : mEntities)
            {
                SceneManager::UnregisterEntity(entityWeakPtr);

            }
            mEntities.clear();
        }
    }

    /// owner
    SceneWeakPtr mOwner;

    /// application reference
    Lumen::Application &mApplication;

    /// entities in the scene
    Entities mEntities;
};

//==============================================================================================================================================================================

/// constructor
Scene::Scene(Lumen::Application &application, const std::filesystem::path &path) :
    Asset(Type(), path), mImpl(Scene::Impl::MakeUniquePtr(application)) {}

/// destroys scene
Scene::~Scene()
{
    Release();
}

/// creates a smart pointer version of the scene
ScenePtr Scene::MakePtr(Lumen::Application &application, const std::filesystem::path &path)
{
    auto ptr = ScenePtr(new Scene(application, path));
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
