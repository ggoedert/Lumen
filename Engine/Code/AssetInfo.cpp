//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFileSystem.h"
#include "lStringMap.h"
#include "lAssetInfo.h"

#include "lMesh.h"

using namespace Lumen;

/// AssetInfo::Impl class
class AssetInfo::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class AssetInfo;

public:
    /// constructs a assetinfo
    explicit Impl(const std::filesystem::path &path) : mPath(path), mUUID(UUID_INVALID) {}

    /// initialize assetinfo
    void Initialize()
    {
        mUUID = UUIDGenerate();
        mType = Mesh::Type();
    }

    /// register assetinfo name / path
    static void Register(std::string_view name, std::string_view path)
    {
        // register asset
        mAssetPaths.insert_or_assign(std::string(name), path);
    }

    /// find assetinfo path from name
    static Expected<std::string_view> Find(std::string_view name)
    {
        // find asset
        auto it = mAssetPaths.find(name);
        if (it != mAssetPaths.end())
        {
            return it->second;
        }

        // none found
        return Expected<std::string_view>::Unexpected(std::format("AssetInfo path for '{}' not found", name));
    }

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        Serialized::SerializeUUID(out, packed, Serialized::cUUIDToken, Serialized::cUUIDTokenPacked, mUUID);
        if (packed)
        {
            Serialized::SerializeValue(out, true, Serialized::cTypeToken, Serialized::cTypeTokenPacked, mType);
        }
        else
        {
#ifdef TYPEINFO
            Serialized::SerializeValue(out, false, Serialized::cTypeToken, Serialized::cTypeTokenPacked, DecodeType(mType));
#else
            Serialized::SerializeValue(out, false, Serialized::cTypeToken, Serialized::cTypeTokenPacked, mType);
#endif
        }
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        Serialized::Type typeValue = {};
        L_ASSERT(Serialized::DeserializeUUID(in, packed, Serialized::cUUIDToken, Serialized::cUUIDTokenPacked, mUUID));
        L_ASSERT(Serialized::DeserializeValue(in, packed, Serialized::cTypeToken, Serialized::cTypeTokenPacked, typeValue));
        if (!typeValue.is_string())
        {
            mType = typeValue;
        }
        else
        {
            mType = EncodeType(typeValue.get<std::string>().c_str());
        }
    }

    /// save assetinfo
    bool Save() const
    {
        std::string infoFile = mPath.string() + std::string(".info");
        Lumen::DebugLog::Info("AssetInfo::Impl::Save {}", infoFile);

        // serialize the assetinfo
        Serialized::Type data;
        Serialize(data, FileSystem::IsPacked(infoFile));

        // write the assetinfo
        return FileSystem::WriteSerializedData(infoFile, data);
    }

    /// load assetinfo
    bool Load()
    {
        std::string infoFile = mPath.string() + std::string(".info");
        Lumen::DebugLog::Info("AssetInfo::Impl::Load {}", infoFile);

        // read the assetinfo
        Serialized::Type data;
        if (!FileSystem::ReadSerializedData(infoFile, data))
        {
            Lumen::DebugLog::Error("Unable to read the assetinfo");
            return false;
        }

        // deserialize the assetinfo
        try
        {
            Deserialize(data, FileSystem::IsPacked(infoFile));
        }
        catch (const std::exception &e)
        {
            Lumen::DebugLog::Error("{}", e.what());
            return false;
        }
        return true;
    }

    /// get path
    const std::filesystem::path &Path() { return mPath; }

    /// get UUID
    const Lumen::UUID UUID() const { return mUUID; }

private:
    /// path
    const std::filesystem::path mPath;

    /// assetinfo UUID
    Lumen::UUID mUUID;

    /// assetinfo type
    Hash mType;

    /// static map of asset names to paths
    static StringMap<std::string> mAssetPaths;
};

StringMap<std::string> AssetInfo::Impl::mAssetPaths;

//==============================================================================================================================================================================

/// constructs a assetinfo
AssetInfo::AssetInfo(const std::filesystem::path &path) : mImpl(AssetInfo::Impl::MakeUniquePtr(path)) {}

/// custom smart pointer maker
Expected<AssetInfoPtr> AssetInfo::MakePtr(const std::filesystem::path &path)
{
    return AssetInfoPtr(new AssetInfo(path));
}

/// initialize assetinfo
void AssetInfo::Initialize()
{
    return mImpl->Initialize();
}

/// register assetinfo name / path
void AssetInfo::Register(std::string_view name, std::string_view path)
{
    AssetInfo::Impl::Register(name, path);
}

/// find assetinfo path from name
Expected<std::string_view> AssetInfo::Find(std::string_view name)
{
    return AssetInfo::Impl::Find(name);
}

/// save assetinfo
bool AssetInfo::Save() const
{
    return mImpl->Save();
}

/// load assetinfo
bool AssetInfo::Load()
{
    return mImpl->Load();
}

/// get path
const std::filesystem::path &AssetInfo::Path() const
{
    return mImpl->Path();
}

/// get UUID
const UUID AssetInfo::UUID() const
{
    return mImpl->UUID();
}
