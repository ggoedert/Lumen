//==============================================================================================================================================================================
/// \file
/// \brief     hashing utilities
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    using Hash = dword;
    using HashToken = std::array<char, 6>;

    constexpr Hash HASH_INVALID = static_cast<Hash>(0);
    constexpr HashToken HASH_TOKEN_INVALID = { 'A', 'A', 'A', 'A', 'A', 'A' };

    constexpr Hash HASH_PRIME = static_cast<Hash>(0x01000193);
    constexpr Hash HASH_OFFSET = static_cast<Hash>(0x811C9DC5);

    /// hash (FNV-1a) range of a string evaluated at compile time
    constexpr Hash HashStringRange(const char *string, size_t begin, size_t end)
    {
        Hash hash = HASH_OFFSET;
        while (begin < end)
        {
            hash ^= *(string + begin);
            hash *= HASH_PRIME;
            begin++;
        }
        return hash;
    }

    /// hash (FNV-1a) string evaluated at compile time
    constexpr Hash HashString(const char *string)
    {
        return HashStringRange(string, 0, std::string_view(string).size());
    }

#ifdef TYPEINFO
    /// typeinfo version of HashType
    struct HashType
    {
        HashType(Hash hash, std::string_view name) : mHash(hash), mName(name) {}
        operator Hash() const { return mHash; }
        bool operator==(HashType &other) const { return mHash == other.mHash; }
        Hash mHash;
        std::string_view mName;
    };
    struct HashTypeHasher
    {
        using is_transparent = void;
        size_t operator()(const HashType &h) const { return static_cast<size_t>(h.mHash); }
        size_t operator()(Hash h) const { return static_cast<size_t>(h); }
    };
    struct HashTypeComparator
    {
        using is_transparent = void;
        bool operator()(const HashType &a, const HashType &b) const { return a.mHash < b.mHash; }
        bool operator()(const HashType &a, Hash b) const { return a.mHash < b; }
        bool operator()(Hash a, const HashType &b) const { return a < b.mHash; }
    };
    struct HashTypeEqual
    {
        using is_transparent = void;
        bool operator()(const HashType &a, const HashType &b) const { return a.mHash == b.mHash; }
        bool operator()(const HashType &a, Hash b) const { return a.mHash == b; }
        bool operator()(Hash a, const HashType &b) const { return a == b.mHash; }
    };

    /// hash (FNV-1a) type from type name, typeinfo version
    HashType EncodeType(const char *typeName);

    /// hash (FNV-1a) type from current function name, typeinfo version
    HashType ClassType(const char *currentFunction);

    /// decode the hash type to a string, typeinfo version
    const char *DecodeType(Hash type);
#else
    /// lean version of HashType
    using HashType = Hash;
    struct HashTypeHasher { size_t operator()(const HashType &h) const { return static_cast<size_t>(h); } };
    struct HashTypeComparator { bool operator()(const HashType &a, const HashType &b) const { return a < b; } };
    struct HashTypeEqual { bool operator()(const HashType &a, const HashType &b) const { return a == b; } };

    /// hash (FNV-1a) type from type name evaluated at compile time
    constexpr HashType EncodeType(const char *typeName)
    {
        return HashType(HashString(typeName));
    }

    /// hash (FNV-1a) type from current function name evaluated at compile time
    consteval HashType ClassType(const char *currentFunction)
    {
        size_t end = std::string_view(currentFunction).find_last_of('(');
        end = std::string_view(currentFunction, end).find_last_of(':') - 1;
        return HashStringRange(currentFunction, std::string_view(currentFunction, end).find_last_of(' ') + 1, end);
    }
#endif

    /// decode the 6 character Base64 array into a 4 byte Hash
    constexpr Hash HashTokenToHash(const HashToken &token)
    {
        if (token.size() != 6) return HASH_INVALID;

        auto val = [](char c) constexpr -> byte
        {
            return (c >= 'A' && c <= 'Z') ? c - 'A' :
                (c >= 'a' && c <= 'z') ? c - 'a' + 26 :
                (c >= '0' && c <= '9') ? c - '0' + 52 :
                (c == '+') ? 62 :
                63;
        };
        byte bytes[4];

        // decode first 4 chars -> 3 bytes
        Hash val24 = (val(token[0]) << 18) | (val(token[1]) << 12) | (val(token[2]) << 6) | val(token[3]);
        bytes[0] = (val24 >> 16) & 0xff;
        bytes[1] = (val24 >> 8) & 0xff;
        bytes[2] = val24 & 0xff;

        // decode last 4 chars -> last byte
        val24 = (val(token[4]) << 6) | val(token[5]);
        bytes[3] = (val24 >> 4) & 0xFF;

        return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    }

    /// encode the 4 byte Hash into an 6 character Base64 array
    constexpr HashToken HashTokenFromHash(const Hash hash)
    {
        const char *table = GetBase64Table();
        HashToken token;

        // encode first 3 bytes -> 4 chars
        Hash val24 = hash >> 8;
        token[0] = table[(val24 >> 18) & 0x3F];
        token[1] = table[(val24 >> 12) & 0x3F];
        token[2] = table[(val24 >> 6) & 0x3F];
        token[3] = table[val24 & 0x3F];

        // encode last byte -> 2 chars
        val24 = (hash & 0xff) << 4;
        token[4] = table[(val24 >> 6) & 0x3F];
        token[5] = table[val24 & 0x3F];

        // return encoded token
        return token;
    }

    /// convert string to 6 character array, returns empty array if string is not 6 characters
    constexpr HashToken HashTokenFromString(const std::string &s)
    {
        return s.size() == 6 ? HashToken { s[0], s[1], s[2], s[3], s[4], s[5] } : HASH_TOKEN_INVALID;
    }
}
