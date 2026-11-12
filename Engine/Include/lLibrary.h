//==============================================================================================================================================================================
/// \file
/// \brief     library interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lUUID.h"
#include "lSerializedData.h"

/// \cond
#include <filesystem>
#include <functional>
/// \endcond

/// Lumen namespace
namespace Lumen
{
    /// Library namespace
    namespace Library
    {
        /// initialize library namespace
        void Initialize(const std::filesystem::path &root);

        /// shutdown library namespace
        void Shutdown();

        /// path of the entry of an asset UUID
        std::filesystem::path EntryPath(UUID uuid);

        /// UUIDs of the entries stored in the Library
        std::vector<UUID> Entries();

        /// reads an entry, only when it is up to date with its source
        bool Read(UUID uuid, const std::filesystem::path &sourcePath, Serialized::Type &data);

        /// reads an entry whatever the state of its source
        bool Read(UUID uuid, Serialized::Type &data);

        /// writes an entry
        bool Write(UUID uuid, const Serialized::Type &data);

        /// removes an entry
        bool Remove(UUID uuid);

        /// removes the entries whose UUID is not alive
        void CollectGarbage(const std::function<bool(UUID uuid)> &isAlive);
    };
}
