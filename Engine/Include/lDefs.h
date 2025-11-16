//==============================================================================================================================================================================
/// \file
/// \brief     engine defines
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDebugLog.h"

#include <string_view>
#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <map>
#include <ranges>
#include <algorithm>
#include <any>
#include <optional>

/// enable type info in debug or editor builds
#if !defined(NDEBUG) || defined(EDITOR)
#define TYPEINFO
#endif

/// Lumen namespace
namespace Lumen
{
    /// typical types
    using byte = uint8_t;
    using word = uint16_t;
    using dword = uint32_t;
    using qword = uint64_t;

    using Hash = dword;

    constexpr Hash HASH_PRIME = static_cast<Hash>(0x01000193);
    constexpr Hash HASH_OFFSET = static_cast<Hash>(0x811C9DC5);
    constexpr Hash HASH_INVALID = static_cast<Hash>(0xFFFFFFFF);

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
#else
    /// lean version of HashType
    using HashType = Hash;
    struct HashTypeHasher { size_t operator()(HashType &h) const { return static_cast<size_t>(h); } };
    struct HashTypeComparator { bool operator()(HashType &a, HashType &b) const { return a < b; } };
    struct HashTypeEqual { bool operator()(HashType &a, HashType &b) const { return a == b; } };

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

    /// encode the 4 byte Hash into an 6 character Base64 string
    constexpr std::string Base64Encode(const Hash hash)
    {
        char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        char token[6];

        // encode first 3 bytes -> 4 chars
        Hash val24 = hash >> 8;
        token[0] = table[(val24 >> 18) & 0x3F];
        token[1] = table[(val24 >> 12) & 0x3F];
        token[2] = table[(val24 >>  6) & 0x3F];
        token[3] = table[val24 & 0x3F];

        // encode last byte -> 2 chars
        val24 = ((hash&0xff) << 4);
        token[4] = table[(val24 >> 6) & 0x3F];
        token[5] = table[val24 & 0x3F];

        // create string of length 6
        return std::string(token, 6);
    }

    /// decode the 8 character Base64 string into a 4 byte Hash
    constexpr Hash Base64Decode(const std::string &token)
    {
        if (token.size() != 6)
        {
            return HASH_INVALID;
        }
        auto val = [](char c) constexpr -> byte
        {
            return (c >= 'A' && c <= 'Z') ? c - 'A' : (c >= 'a' && c <= 'z') ? c - 'a' + 26 : (c >= '0' && c <= '9') ? c - '0' + 52 : (c == '+') ? 62 : 63;
        };
        byte bytes[4];

        // decode first 4 chars -> 3 bytes
        Hash val24 = (val(token[0]) << 18) | (val(token[1]) << 12) | (val(token[2]) << 6) | val(token[3]);
        bytes[0] = (val24 >> 16) & 0xff;
        bytes[1] = (val24 >> 8) & 0xff;
        bytes[2] = val24 & 0xff;

        // decode last 4 chars -> last byte
        val24 = (val(token[4]) << 18) | (val(token[5]) << 12);
        bytes[3] = (val24 >> 16) & 0xFF;

        return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    }

    /// class name from current function name evaluated at compile time
    consteval std::string_view ClassName(const char *currentFunction)
    {
        size_t end = std::string_view(currentFunction).find_last_of('(');
        end = std::string_view(currentFunction, end).find_last_of(':') - 1;
        size_t begin = std::string_view(currentFunction, end).find_last_of(' ') + 1;
        return std::string_view(currentFunction + begin, end - begin);
    }

    /// remove from vector if predicate is true
    template<typename T, typename Predicate>
    bool RemoveFromVectorIf(std::vector<T> &vec, Predicate pred)
    {
        auto newEnd = std::remove_if(vec.begin(), vec.end(), pred);
        bool wasRemoved = (newEnd != vec.end());
        if (wasRemoved)
        {
            vec.erase(newEnd, vec.end());
        }
        return wasRemoved;
    }

    /// remove from vector by value
    template<typename T, typename U>
    bool RemoveFromVector(std::vector<T> &vec, const U &value)
    {
        return RemoveFromVectorIf(vec, [&value](const T &element) { return element == value; });
    }
}

/// disable warning helpers
#if defined(_MSC_VER)
#define WARNING_DISABLE_NODISCARD_PUSH() \
do                                       \
{                                        \
    __pragma(warning(push))              \
    __pragma(warning(disable: 4834))     \
} while (false)
#define WARNING_POP() do { __pragma(warning(pop)) } while (false)
#elif defined(__GNUC__)
#define WARNING_DISABLE_NODISCARD_PUSH()                  \
do                                                        \
{                                                         \
    _Pragma("GCC diagnostic push")                        \
    _Pragma("GCC diagnostic ignored \"-Wunused-result\"") \
} while (false)
#define WARNING_POP() do { _Pragma("GCC diagnostic pop") } while (false)
#elif defined(__clang__)
#define WARNING_DISABLE_NODISCARD_PUSH()                    \
do                                                          \
{                                                           \
    _Pragma("clang diagnostic push")                        \
    _Pragma("clang diagnostic ignored \"-Wunused-result\"") \
} while (false)
#define WARNING_POP() do { _Pragma("clang diagnostic pop") } while (false)
#else
#define WARNING_DISABLE_NODISCARD_PUSH() do {} while (false)
#define WARNING_POP() do {} while (false)
#endif

/// current enclosing function
#if defined(__INTELLISENSE__)
#define CURRENT_FUNCTION "HashType Class::Method()"
#elif defined(__FUNCSIG__)
#define CURRENT_FUNCTION __FUNCSIG__
#elif defined(__GNUC__) ||  defined(__clang__)
#define CURRENT_FUNCTION __PRETTY_FUNCTION__
#else
#error Unable to determine the current function.
#endif

#define L_STRINGIZE_HELP(EXPR) #EXPR
#define L_STRINGIZE(EXPR) L_STRINGIZE_HELP(EXPR)

#ifdef NDEBUG
#define L_ASSERT(...) ((void)0)
#define L_ASSERT_MSG(...) ((void)0)
#else
#define L_ASSERT(EXPR)                                                                                         \
do {                                                                                                           \
    if (!(EXPR))                                                                                               \
    {                                                                                                          \
        Lumen::DebugLog::Error("{}", "(Assertion failed) " #EXPR " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                                          \
    }                                                                                                          \
} while (false)
#define L_ASSERT_MSG(EXPR, ...)                                                                                                                  \
do {                                                                                                                                             \
    if (!(EXPR))                                                                                                                                 \
    {                                                                                                                                            \
        Lumen::DebugLog::Error("{}", "(Assertion failed) " #EXPR ", " + std::format(__VA_ARGS__) + " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                                                                            \
    }                                                                                                                                            \
} while (false)
#endif
#define L_CHECK(EXPR)                                                                                      \
do {                                                                                                       \
    if (!(EXPR))                                                                                           \
    {                                                                                                      \
        Lumen::DebugLog::Error("{}", "(Check failed) " #EXPR " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                                      \
    }                                                                                                      \
} while (false)
#define L_CHECK_MSG(EXPR, ...)                                                                                                               \
do {                                                                                                                                         \
    if (!(EXPR))                                                                                                                             \
    {                                                                                                                                        \
        Lumen::DebugLog::Error("{}", "(Check failed) " #EXPR ", " + std::format(__VA_ARGS__) + " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                                                                        \
    }                                                                                                                                        \
} while (false)
#define L_VERIFY(EXPR)                                                                                      \
do {                                                                                                        \
    if (!(EXPR))                                                                                            \
    {                                                                                                       \
        Lumen::DebugLog::Error("{}", "(Verify failed) " #EXPR " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
    }                                                                                                       \
} while (false)
#define L_VERIFY_MSG(EXPR, ...)                                                                                                               \
do {                                                                                                                                          \
    if (!(EXPR))                                                                                                                              \
    {                                                                                                                                         \
        Lumen::DebugLog::Error("{}", "(Verify failed) " #EXPR ", " + std::format(__VA_ARGS__) + " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
    }                                                                                                                                         \
} while (false)

#define CLASS_NO_DEFAULT_CTOR(TYPE) \
public:                             \
TYPE() = delete

#define CLASS_NO_COPY(TYPE)            \
public:                                \
TYPE(const TYPE &) = delete;           \
TYPE &operator=(const TYPE &) = delete

#define CLASS_NO_MOVE(TYPE)       \
public:                           \
TYPE(TYPE &&) = delete;           \
TYPE &operator=(TYPE &&) = delete

#define CLASS_NO_COPY_MOVE(TYPE)        \
CLASS_NO_COPY(TYPE);                    \
CLASS_NO_MOVE(TYPE)

#define CLASS_PTR_DEF(TYPE)             \
class TYPE;                             \
using TYPE##Ptr = std::shared_ptr<TYPE>

#define CLASS_WEAK_PTR_DEF(TYPE)          \
class TYPE;                               \
using TYPE##WeakPtr = std::weak_ptr<TYPE>

#define CLASS_UNIQUE_PTR_DEF(TYPE)            \
class TYPE;                                   \
using TYPE##UniquePtr = std::unique_ptr<TYPE>

#define CLASS_PIMPL_DEF(TYPE) \
CLASS_UNIQUE_PTR_DEF(TYPE);   \
TYPE##UniquePtr m##TYPE

#define CLASS_PTR_MAKER(TYPE)                                                                                \
public:                                                                                                      \
template <typename...Args>                                                                                   \
inline static TYPE##Ptr MakePtr(Args&&... args) { return TYPE##Ptr(new TYPE(std::forward<Args>(args)...)); }

#define CLASS_PTR_UNIQUEMAKER(TYPE)                                                                                        \
public:                                                                                                                    \
template <typename...Args>                                                                                                 \
inline static TYPE##UniquePtr MakeUniquePtr(Args&&...args) { return std::make_unique<TYPE>(std::forward<Args>(args)...); }

#define CLASS_PIMPL_MAKER(TYPE)                                                                                           \
public:                                                                                                                   \
template <typename...Args>                                                                                                \
inline static TYPE##UniquePtr MakeUniquePtr(Args&&...args) { return std::make_unique<TYPE>(std::forward<Args>(args)...); }

#ifdef TYPEINFO
#define TYPE_METHOD static const Lumen::HashType Type() { return Lumen::ClassType(CURRENT_FUNCTION); }
#else
#define TYPE_METHOD static constexpr Lumen::HashType Type() { return Lumen::ClassType(CURRENT_FUNCTION); }
#endif

#define OBJECT_TYPEINFO \
public:                 \
TYPE_METHOD

#define COMPONENT_TYPEINFO                                                                   \
public:                                                                                      \
TYPE_METHOD                                                                                  \
static std::string_view Name() { return mName; }                                             \
private:                                                                                     \
static consteval std::string_view CacheName() { return Lumen::ClassName(CURRENT_FUNCTION); } \
static const std::string mName;                                                              \
static bool Register();                                                                      \
static const bool mRegistered

#define DEFINE_COMPONENT_TYPEINFO(TYPE)                                                                          \
const std::string TYPE::mName = std::string(TYPE::CacheName());                                                  \
const bool TYPE::mRegistered = TYPE::Register();                                                                 \
bool TYPE::Register() { Lumen::SceneManager::RegisterComponentMaker(TYPE::Type(), TYPE::MakePtr); return true; }
