//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssetInfo.h"
#include "lFileSystem.h"

using namespace Lumen;

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    /// asset type of a stored type hash
    inline HashType TypeOfHash(Hash hash)
    {
#ifdef TYPEINFO
        return HashType(hash, DecodeType(hash));
#else
        return hash;
#endif
    }
}

/// AssetInfo::Impl class
class AssetInfo::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class AssetInfo;

public:
    /// constructs a assetinfo
    explicit Impl(const std::filesystem::path &path) : mPath(path), mUUID(UUID_INVALID) {}

    /// constructs a builtin assetinfo, all of its data comes from code
    explicit Impl(const std::filesystem::path &path, HashType type, Lumen::UUID uuid) :
        mPath(path), mUUID(uuid), mBuiltin(true), mType(static_cast<Hash>(type)) {}

    /// initialize assetinfo
    void Initialize()
    {
        L_ASSERT(!mBuiltin);
        mUUID = UUIDGenerate();
    }

    /// get path
    const std::filesystem::path &Path() const { return mPath; }

    /// get UUID
    const Lumen::UUID UUID() const { return mUUID; }

    /// whether the assetinfo is the one of a builtin asset
    bool Builtin() const { return mBuiltin; }

    /// get the asset type
    HashType Type() const { return Hidden::TypeOfHash(mType); }

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        // \todo add serialize per type array of properties
        out = Serialized::Type::object();
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        // \todo add deserialize per type array of properties
    }

    /// save assetinfo
    bool Save() const
    {
        if (mBuiltin)
        {
            Lumen::DebugLog::Error("Builtin assetinfo {} is defined in code, it has no info file", mPath.generic_string());
            return false;
        }

        std::filesystem::path infoFile = std::filesystem::path(mPath).concat(".info");
        Lumen::DebugLog::Info("AssetInfo::Impl::Save {}", infoFile.generic_string());

        // serialize the assetinfo
        Serialized::Type data;
        Serialize(data, FileSystem::IsPacked(infoFile));

        // write the assetinfo
        return FileSystem::WriteAssetInfo(infoFile, mUUID, data);
    }

    /// load assetinfo
    bool Load()
    {
        if (mBuiltin)
        {
            Lumen::DebugLog::Error("Builtin assetinfo {} is defined in code, it has no info file", mPath.generic_string());
            return false;
        }

        std::filesystem::path infoFile = std::filesystem::path(mPath).concat(".info");
        Lumen::DebugLog::Info("AssetInfo::Impl::Load {}", infoFile.generic_string());

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

    /// whether the assetinfo is the one of a builtin asset
    const bool mBuiltin = false;

    /// asset type, only stored for builtin assets
    const Hash mType = HASH_INVALID;
};

//==============================================================================================================================================================================

/// constructs a assetinfo
AssetInfo::AssetInfo(const std::filesystem::path &path) : mImpl(AssetInfo::Impl::MakeUniquePtr(path)) {}

/// constructs a builtin assetinfo
AssetInfo::AssetInfo(const std::filesystem::path &path, HashType type, Lumen::UUID uuid) : mImpl(AssetInfo::Impl::MakeUniquePtr(path, type, uuid)) {}

/// destructor
AssetInfo::~AssetInfo() = default;

/// creates a smart pointer version of the AssetInfo (custom deletion)
AssetInfoPtr AssetInfo::MakePtr(const std::filesystem::path &path, Deleter deleter)
{
    return AssetInfoPtr(new AssetInfo(path), std::move(deleter));
}

/// creates a smart pointer version of the AssetInfo of a builtin asset (custom deletion)
AssetInfoPtr AssetInfo::MakeBuiltinPtr(const std::filesystem::path &path, HashType type, Lumen::UUID uuid, Deleter deleter)
{
    return AssetInfoPtr(new AssetInfo(path, type, uuid), std::move(deleter));
}

/// whether a path names a builtin asset
bool AssetInfo::IsBuiltinPath(const std::filesystem::path &path)
{
    return path.generic_string().starts_with('|');
}

/// initialize assetinfo
void AssetInfo::Initialize()
{
    mImpl->Initialize();
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
Lumen::UUID AssetInfo::UUID() const
{
    return mImpl->UUID();
}

/// whether the assetinfo is the one of a builtin asset
bool AssetInfo::Builtin() const
{
    return mImpl->Builtin();
}

/// get the asset type
HashType AssetInfo::Type() const
{
    return mImpl->Type();
}
