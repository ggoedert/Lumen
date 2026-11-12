//==============================================================================================================================================================================
/// \file
/// \brief     Library
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lLibrary.h"
#include "lFileSystem.h"

/// \cond
#include <charconv>
/// \endcond

using namespace Lumen;

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    CLASS_UNIQUE_PTR_DEF(LibraryImpl);

    /// LibraryImpl class
    class LibraryImpl
    {
        CLASS_NO_DEFAULT_CTOR(LibraryImpl);
        CLASS_NO_COPY_MOVE(LibraryImpl);
        CLASS_PTR_UNIQUEMAKER(LibraryImpl);

    public:
        /// constructs a library implementation with its root folder
        explicit LibraryImpl(const std::filesystem::path &root) : mRoot(FileSystem::NormalizeDirPath(root).generic_string()) {}

        /// destroys library
        ~LibraryImpl() = default;

        /// path of the entry of an asset UUID
        std::filesystem::path EntryPath(UUID uuid) const
        {
            return mRoot + UUIDToString(uuid);
        }

        /// UUIDs of the entries stored in the Library
        std::vector<UUID> Entries() const
        {
            std::vector<UUID> uuids;
            for (const auto &file : FileSystem::ListFiles(mRoot))
            {
                UUID uuid = UUIDFromString(file.mName);
                if (file.mAttributes.mFlags.Has(FileSystem::Flag::File) && (uuid != UUID_INVALID))
                {
                    uuids.push_back(uuid);
                }
            }
            return uuids;
        }

        /// reads an entry, only when it is up to date with its source
        bool Read(UUID uuid, const std::filesystem::path &sourcePath, Serialized::Type &data) const
        {
            // a missing entry is a normal cache miss, stay silent
            const FileSystem::FileAttributes entry = FileSystem::GetAttributes(EntryPath(uuid));
            if (!entry.mFlags.Has(FileSystem::Flag::File))
            {
                return false;
            }

            // up to date only when strictly newer than the source, equal times (coarse file systems) rebuild to be safe
            const FileSystem::FileAttributes source = FileSystem::GetAttributes(sourcePath);
#ifdef EDITOR
            // in the editor the source is the truth, a missing source is always out of date
            bool outOfDate = source.mFlags.Has(FileSystem::Flag::File) ? (entry.mLastWrite < source.mLastWrite) : true;
#else
            // in a game build the source may not exist, a missing source is never out of date (the Library is the truth)
            bool outOfDate = source.mFlags.Has(FileSystem::Flag::File) ? (entry.mLastWrite < source.mLastWrite) : false;
#endif
            if (outOfDate)
            {
                Lumen::DebugLog::Detail("Library entry of {} is out of date", sourcePath.generic_string());
                return false;
            }
            return Read(uuid, data);
        }

        /// reads an entry whatever the state of its source
        bool Read(UUID uuid, Serialized::Type &data) const
        {
            return FileSystem::ReadSerializedData(EntryPath(uuid), data);
        }

        /// writes an entry
        bool Write(UUID uuid, const Serialized::Type &data) const
        {
            return FileSystem::WriteSerializedData(EntryPath(uuid), data);
        }

        /// removes an entry
        bool Remove(UUID uuid) const
        {
            const std::filesystem::path entryPath = EntryPath(uuid);
            return FileSystem::Exists(entryPath) && FileSystem::Delete(entryPath);
        }

        /// removes the entries whose UUID is not alive
        void CollectGarbage(const std::function<bool(UUID uuid)> &isAlive) const
        {
            const size_t removed = RemoveEntries([&isAlive](UUID uuid) { return !isAlive(uuid); });
            if (removed > 0)
            {
                Lumen::DebugLog::Info("Removed {} unused Library entries", removed);
            }
        }

    private:
        /// removes the entries the predicate selects, returns how many were removed
        size_t RemoveEntries(const std::function<bool(UUID uuid)> &remove) const
        {
            size_t removed = 0;
            for (UUID uuid : Entries())
            {
                if (remove(uuid))
                {
                    removed += FileSystem::Delete(EntryPath(uuid)) ? 1 : 0;
                }
            }
            return removed;
        }

        /// root folder, normalized with a trailing slash
        const std::string mRoot;
    };

    /// global library implementation pointer
    static LibraryImplUniquePtr gLibraryImpl;
}

//==============================================================================================================================================================================

/// initialize library namespace
void Library::Initialize(const std::filesystem::path &root)
{
    L_ASSERT(!Hidden::gLibraryImpl);
    Hidden::gLibraryImpl = Hidden::LibraryImpl::MakeUniquePtr(root);
}

/// shutdown library namespace
void Library::Shutdown()
{
    L_ASSERT(Hidden::gLibraryImpl);
    Hidden::gLibraryImpl.reset();
}

/// path of the entry of an asset UUID
std::filesystem::path Library::EntryPath(UUID uuid)
{
    L_ASSERT(Hidden::gLibraryImpl);
    return Hidden::gLibraryImpl->EntryPath(uuid);
}

/// UUIDs of the entries stored in the Library
std::vector<UUID> Library::Entries()
{
    L_ASSERT(Hidden::gLibraryImpl);
    return Hidden::gLibraryImpl->Entries();
}

/// reads an entry, only when it is up to date with its source
bool Library::Read(UUID uuid, const std::filesystem::path &sourcePath, Serialized::Type &data)
{
    L_ASSERT(Hidden::gLibraryImpl);
    return Hidden::gLibraryImpl->Read(uuid, sourcePath, data);
}

/// reads an entry whatever the state of its source
bool Library::Read(UUID uuid, Serialized::Type &data)
{
    L_ASSERT(Hidden::gLibraryImpl);
    return Hidden::gLibraryImpl->Read(uuid, data);
}

/// writes an entry
bool Library::Write(UUID uuid, const Serialized::Type &data)
{
    L_ASSERT(Hidden::gLibraryImpl);
    return Hidden::gLibraryImpl->Write(uuid, data);
}

/// removes an entry
bool Library::Remove(UUID uuid)
{
    L_ASSERT(Hidden::gLibraryImpl);
    return Hidden::gLibraryImpl->Remove(uuid);
}

/// removes the entries whose UUID is not alive
void Library::CollectGarbage(const std::function<bool(UUID uuid)> &isAlive)
{
    L_ASSERT(Hidden::gLibraryImpl);
    Hidden::gLibraryImpl->CollectGarbage(isAlive);
}
