//==============================================================================================================================================================================
/// \file
/// \brief     UUID functions for asset management and other unique identifiers
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    /// UUID type, used for asset management
    using UUID = qword;

    /// size of the string representation of a UUID
    constexpr int UUID_STRING_SIZE = 16;

    /// invalid UUID, used to indicate an uninitialized or invalid state
    constexpr UUID UUID_INVALID = static_cast<UUID>(0);

    /// last UUID of the range reserved for the builtin assets, the range starts right after UUID_INVALID
    constexpr UUID UUID_BUILTIN_LAST = static_cast<UUID>(0xf);

    /// fast static generator
    UUID UUIDGenerate();

    /// whether a UUID is in the range reserved for the builtin assets
    constexpr bool UUIDIsBuiltin(UUID id) { return (id != UUID_INVALID) && (id <= UUID_BUILTIN_LAST); }

    /// serialize to string
    constexpr std::string UUIDToString(UUID id)
    {
        std::string result(UUID_STRING_SIZE, '0');
        for (int i = (UUID_STRING_SIZE-1); id; --i, id >>= 4)
        {
            result[i] = "0123456789abcdef"[id & 0xf];
        }
        return result;
    }

    /// deserialize from string
    constexpr UUID UUIDFromString(const std::string &str)
    {
        UUID uuid = UUID_INVALID;
        if ((str.size() == UUID_STRING_SIZE) && std::all_of(str.begin(), str.end(), [](char c) { return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')); }))
        {
            std::from_chars(str.data(), str.data() + str.size(), uuid, UUID_STRING_SIZE);
        }
        return uuid;
    }
}
