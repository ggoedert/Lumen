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
    explicit Impl(const std::filesystem::path &path) : mPath(path), mUUID(UUID_INVALID) {}

    /// initialize assetinfo
    void Initialize()
    {
        mUUID = UUIDGenerate();
    }

    /// get path
    const std::filesystem::path &Path() { return mPath; }

    /// get UUID
    const Lumen::UUID UUID() const { return mUUID; }

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
};

//==============================================================================================================================================================================

/// constructs a assetinfo
AssetInfo::AssetInfo(const std::filesystem::path &path) : mImpl(AssetInfo::Impl::MakeUniquePtr(path)) {}

/// destructor
AssetInfo::~AssetInfo() = default;

/// creates a smart pointer version of the AssetInfo (custom deletion)
AssetInfoPtr AssetInfo::MakePtr(const std::filesystem::path &path, Deleter deleter)
{
    return AssetInfoPtr(new AssetInfo(path), std::move(deleter));
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
const UUID AssetInfo::UUID() const
{
    return mImpl->UUID();
}
