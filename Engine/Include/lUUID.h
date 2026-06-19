//==============================================================================================================================================================================
/// \file
/// \brief     UUID functions for asset management and other unique identifiers
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lHash.h"

/// Lumen namespace
namespace Lumen
{
    using UUID = qword;

    constexpr UUID UUID_INVALID = static_cast<UUID>(0);

    /// fast static generator
    UUID UUIDGenerate();

    /// serialize to string
    std::string UUIDToString(const UUID id);

    /// deserialize from string
    constexpr UUID UUIDFromString(const std::string &str)
    {
        return str.size() == 12 ? static_cast<Lumen::UUID>(HashTokenToHash(HashToken { str[0], str[1], str[2], str[3], str[4], str[5] })) |
            (static_cast<Lumen::UUID>(HashTokenToHash(HashToken { str[6], str[7], str[8], str[9], str[10], str[11] })) << (sizeof(Hash) * CHAR_BIT)) : UUID_INVALID;
    }
}
