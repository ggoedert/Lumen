//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssetInfo.h"
#include "lFileSystem.h"

using namespace Lumen;

/// AssetInfo::Impl class
class AssetInfo::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class AssetInfo;

public:
    /// constructs a assetinfo
    explicit Impl(const std::filesystem::path &path) : mPath(path), mUUID(UUID_INVALID), mType(HASH_INVALID) {}

    /// initialize assetinfo
    void Initialize()
    {
        mUUID = UUIDGenerate();
        mType = HASH_INVALID;  // TODO use path to determine type

        // register assetinfo in caches
        mAssetPathCache[mUUID] = mPath;
        mAssetUUIDCache[mPath] = mUUID;
    }

    /// get or create a smart pointer version of the assetinfo from smart pointer cache
    static Expected<AssetInfoPtr> AcquirePtr(const std::filesystem::path &path)
    {
        auto it = mAssetInfoCache.find(path);
        if (it != mAssetInfoCache.end())
        {
            if (auto ptr = it->second.lock())
            {
                return ptr;
            }
            mAssetInfoCache.erase(it);
        }

        // custom deleter, cleans up the cache when the last AssetInfoPtr is destroyed
        auto deleter = [path](AssetInfo *ptr)
        {
            delete ptr;
            mAssetInfoCache.erase(path);
        };
        auto assetInfoPtr = AssetInfoPtr(new AssetInfo(path), deleter);

        std::filesystem::path infoFile = std::filesystem::path(path).concat(".info");
        if (!FileSystem::Exists(infoFile))
        {
            assetInfoPtr->mImpl->Initialize();
            if (!assetInfoPtr->mImpl->Save())
            {
                return Expected<AssetInfoPtr>::Unexpected("Failed to save asset info");
            }
            DebugLog::Info("Created asset info file in Assets folder: {}", infoFile.string());
        }
        else
        {
            if (!assetInfoPtr->mImpl->Load())
            {
                return Expected<AssetInfoPtr>::Unexpected("Failed to load asset info");
            }
        }

        mAssetInfoCache[path] = assetInfoPtr;
        return assetInfoPtr;
    }

    /// register asset info
    static void Register(const std::filesystem::path &path)
    {
        auto UUIDCacheIt = mAssetUUIDCache.find(path);
        if (UUIDCacheIt == mAssetUUIDCache.end())
        {
            Lumen::UUID uuid;
            if (FileSystem::ReadAssetInfo(std::filesystem::path(path).concat(".info"), uuid, nullptr))
            {
                mAssetPathCache[uuid] = path;
                mAssetUUIDCache[path] = uuid;
            }
        }
    }

    /// rename asset info
    static void Rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
    {
        // find old path in cache
        auto UUIDCacheIt = mAssetUUIDCache.find(oldPath);
        if (UUIDCacheIt != mAssetUUIDCache.end())
        {
            // save mapped uuid and erase old path from cache
            Lumen::UUID uuid = UUIDCacheIt->second;
            mAssetUUIDCache.erase(UUIDCacheIt);

            // find uuid in cache
            auto pathCacheIt = mAssetPathCache.find(uuid);
            if (pathCacheIt != mAssetPathCache.end())
            {
                // update mapped path and insert uuid to cache
                pathCacheIt->second = newPath;
                mAssetUUIDCache[newPath] = uuid;
            }
        }
    }

    /// remove asset info
    static void Remove(const std::filesystem::path &path)
    {
        // find path in cache
        auto UUIDCacheIt = mAssetUUIDCache.find(path);
        if (UUIDCacheIt != mAssetUUIDCache.end())
        {
            // erase uuid and path from cache
            mAssetPathCache.erase(UUIDCacheIt->second);
            mAssetUUIDCache.erase(UUIDCacheIt);
        }
    }

    /// get path
    const std::filesystem::path &Path() { return mPath; }

    /// get UUID
    const Lumen::UUID UUID() const { return mUUID; }

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
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
        std::filesystem::path infoFile = std::filesystem::path(mPath).concat(".info");
        Lumen::DebugLog::Info("AssetInfo::Impl::Save {}", infoFile.string());

        // serialize the assetinfo
        Serialized::Type data;
        Serialize(data, FileSystem::IsPacked(infoFile));

        // write the assetinfo
        return FileSystem::WriteAssetInfo(infoFile, mUUID, data);
    }

    /// load assetinfo
    bool Load()
    {
        std::filesystem::path infoFile = std::filesystem::path(mPath).concat(".info");
        Lumen::DebugLog::Info("AssetInfo::Impl::Load {}", infoFile.string());

        // read the assetinfo
        Serialized::Type data;
        if (!FileSystem::ReadAssetInfo(infoFile, mUUID, &data))
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

private:
    /// path
    const std::filesystem::path mPath;

    /// assetinfo UUID
    Lumen::UUID mUUID;

    /// assetinfo type
    Hash mType;

    /// AssetInfo cache
    static std::unordered_map<std::filesystem::path, AssetInfoWeakPtr> mAssetInfoCache;

    /// AssetInfo path cache
    static std::unordered_map<Lumen::UUID, std::filesystem::path> mAssetPathCache;

    /// AssetInfo uuid cache
    static std::unordered_map<std::filesystem::path, Lumen::UUID> mAssetUUIDCache;
};

/// AssetInfo cache
std::unordered_map<std::filesystem::path, AssetInfoWeakPtr> AssetInfo::Impl::mAssetInfoCache;

/// AssetInfo path cache
std::unordered_map<Lumen::UUID, std::filesystem::path> AssetInfo::Impl::mAssetPathCache;

/// AssetInfo uuid cache
std::unordered_map<std::filesystem::path, Lumen::UUID> AssetInfo::Impl::mAssetUUIDCache;

//==============================================================================================================================================================================

/// constructs a assetinfo
AssetInfo::AssetInfo(const std::filesystem::path &path) : mImpl(AssetInfo::Impl::MakeUniquePtr(path)) {}

/// get or create a smart pointer version of the assetinfo from smart pointer cache
Expected<AssetInfoPtr> AssetInfo::AcquirePtr(const std::filesystem::path &path)
{
    return Impl::AcquirePtr(path);
}

/// register asset info
void AssetInfo::Register(const std::filesystem::path &path)
{
    return Impl::Register(path);
}

/// rename asset info
void AssetInfo::Rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
{
    return Impl::Rename(oldPath, newPath);
}

/// remove asset info
void AssetInfo::Remove(const std::filesystem::path &path)
{
    return Impl::Remove(path);
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
