//==============================================================================================================================================================================
/// \file
/// \brief     FileInfo
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lFileInfo.h"
#include "lFileSystem.h"

using namespace Lumen;

/// FileInfo::Impl class
class FileInfo::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class FileInfo;

public:
    /// constructs a fileinfo
    explicit Impl(const std::filesystem::path &path, HashType type, Lumen::UUID uuid) : mPath(path), mType(type), mUUID(uuid) {}

    /// initialize fileinfo
    void Initialize()
    {
        mUUID = UUIDGenerate();
    }

    /// get path
    const std::filesystem::path &Path() const { return mPath; }

    /// get UUID
    const Lumen::UUID UUID() const { return mUUID; }

    /// get the asset type
    HashType Type() const { return mType; }

    /// serialize the content of the info file
    void Serialize(Serialized::Type &out, bool packed) const
    {
        // \todo add serialize per type array of properties
        out = Serialized::Type::object();
    }

    /// deserialize the content of the info file
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        // \todo add deserialize per type array of properties
    }

    /// save fileinfo
    bool Save() const
    {
        std::filesystem::path infoFile = std::filesystem::path(mPath).concat(".info");
        Lumen::DebugLog::Info("FileInfo::Impl::Save {}", infoFile.generic_string());

        // serialize the fileinfo
        Serialized::Type data;
        Serialize(data, FileSystem::IsPacked(infoFile));

        // write the fileinfo
        return FileSystem::WriteAssetInfo(infoFile, mUUID, data);
    }

    /// load fileinfo
    bool Load()
    {
        std::filesystem::path infoFile = std::filesystem::path(mPath).concat(".info");
        Lumen::DebugLog::Info("FileInfo::Impl::Load {}", infoFile.generic_string());

        // read the fileinfo
        Serialized::Type data;
        if (!FileSystem::ReadAssetInfo(infoFile, mUUID, &data))
        {
            Lumen::DebugLog::Error("Unable to read the fileinfo");
            return false;
        }

        // deserialize the fileinfo
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

    /// asset type
    const HashType mType;

    /// fileinfo UUID
    Lumen::UUID mUUID;
};

//==============================================================================================================================================================================

/// constructs a fileinfo
FileInfo::FileInfo(const std::filesystem::path &path, HashType type, Lumen::UUID uuid) : mImpl(FileInfo::Impl::MakeUniquePtr(path, type, uuid)) {}

/// destructor
FileInfo::~FileInfo() = default;

/// creates a smart pointer version of the FileInfo (custom deletion)
FileInfoPtr FileInfo::MakePtr(const std::filesystem::path &path, HashType type, Deleter deleter)
{
    return FileInfoPtr(new FileInfo(path, type, UUID_INVALID), std::move(deleter));
}

/// creates a smart pointer version of the FileInfo from a serialized asset info (custom deletion)
Expected<FileInfoPtr> FileInfo::Deserialize(const Serialized::Type &in, bool packed, Deleter deleter)
{
    Lumen::UUID uuid = UUID_INVALID;
    std::filesystem::path path;
    Hash type = HASH_INVALID;
    if (!AssetInfo::Deserialize(in, packed, uuid, path, type))
    {
        return Expected<FileInfoPtr>::Unexpected("Serialized asset info is incomplete");
    }
    return FileInfoPtr(new FileInfo(path, GetHashType(type), uuid), std::move(deleter));
}

/// initialize fileinfo
void FileInfo::Initialize()
{
    mImpl->Initialize();
}

/// save fileinfo
bool FileInfo::Save() const
{
    return mImpl->Save();
}

/// load fileinfo
bool FileInfo::Load()
{
    return mImpl->Load();
}

/// get path
const std::filesystem::path &FileInfo::Path() const
{
    return mImpl->Path();
}

/// get UUID
Lumen::UUID FileInfo::UUID() const
{
    return mImpl->UUID();
}

/// get the asset type
HashType FileInfo::Type() const
{
    return mImpl->Type();
}
