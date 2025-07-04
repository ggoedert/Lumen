//==============================================================================================================================================================================
/// \file
/// \brief     engine defines
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

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

#include "lDebugLog.h"

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

    /// hash (FNV-1a) range of a string evaluated at compile time
    consteval Hash HashStringRange(const char *string, size_t begin, size_t end)
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
    consteval Hash HashString(const char *string)
    {
        return HashStringRange(string, 0, std::string_view(string).size());
    }

#ifdef TYPEINFO
    /// typeinfo version of HashType
    struct HashType
    {
        HashType(Hash hash, std::string_view name) : mHash(hash), mName(name) {}
        operator Hash() const { return mHash; }
        bool operator==(const HashType &other) const { return mHash == other.mHash; }
        Hash mHash;
        std::string_view mName;
    };

    /// hash (FNV-1a) name from current type, typeinfo version
    HashType PodType(const char *currentType);

    /// hash (FNV-1a) class name from current function name, typeinfo version
    HashType ClassType(const char *currentFunction);
#else
    /// lean version of HashType
    using HashType = Hash;

    /// hash (FNV-1a) name from current type evaluated at compile time
    consteval HashType PodType(const char *currentType)
    {
        return HashType(HashString(currentType));
    }

    /// hash (FNV-1a) class name from current function name evaluated at compile time
    consteval HashType ClassType(const char *currentFunction)
    {
        size_t end = std::string_view(currentFunction).find_last_of('(');
        end = std::string_view(currentFunction, end).find_last_of(':') - 1;
        return HashStringRange(currentFunction, std::string_view(currentFunction, end).find_last_of(' ') + 1, end);
    }
#endif

    /// class name from current function name evaluated at compile time
    consteval std::string_view ClassName(const char *currentFunction)
    {
        size_t end = std::string_view(currentFunction).find_last_of('(');
        end = std::string_view(currentFunction, end).find_last_of(':') - 1;
        size_t begin = std::string_view(currentFunction, end).find_last_of(' ') + 1;
        return std::string_view(currentFunction + begin, end - begin);
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
#elif defined(__PRETTY_FUNCTION__)
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
#define L_ASSERT(EXPR)                                                                                  \
do {                                                                                                    \
    if (!(EXPR)) {                                                                                      \
        Lumen::DebugLog::Error("Assertion failed: " #EXPR " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                                   \
    }                                                                                                   \
} while (false)
#define L_ASSERT_MSG(EXPR, MSG)                                                                                                   \
do {                                                                                                                              \
    if (!(EXPR)) {                                                                                                                \
        Lumen::DebugLog::Error("Assertion failed: " #EXPR ", " + std::string(MSG) + " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                                                             \
    }                                                                                                                             \
} while (false)
#endif
#define L_CHECK(EXPR)                                                                               \
do {                                                                                                \
    if (!(EXPR)) {                                                                                  \
        Lumen::DebugLog::Error("Check failed: " #EXPR " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                               \
    }                                                                                               \
} while (false)
#define L_CHECK_MSG(EXPR, MSG)                                                                                                \
do {                                                                                                                          \
    if (!(EXPR)) {                                                                                                            \
        Lumen::DebugLog::Error("Check failed: " #EXPR ", " + std::string(MSG) + " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
        std::abort();                                                                                                         \
    }                                                                                                                         \
} while (false)
#define L_VERIFY(EXPR)                                                                               \
do {                                                                                                 \
    if (!(EXPR)) {                                                                                   \
        Lumen::DebugLog::Error("Verify failed: " #EXPR " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
    }                                                                                                \
} while (false)
#define L_VERIFY_MSG(EXPR, MSG)                                                                                                \
do {                                                                                                                           \
    if (!(EXPR)) {                                                                                                             \
        Lumen::DebugLog::Error("Verify failed: " #EXPR ", " + std::string(MSG) + " [" __FILE__ ":" L_STRINGIZE(__LINE__) "]"); \
    }                                                                                                                          \
} while (false)

#define CLASS_NO_DEFAULT_CTOR(TYPE) \
public:                             \
TYPE() = delete

#define CLASS_NO_COPY_MOVE(TYPE)        \
public:                                 \
TYPE(const TYPE &) = delete;            \
TYPE &operator=(const TYPE &) = delete; \
TYPE(TYPE &&) = delete;                 \
TYPE &operator=(TYPE &&) = delete

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
#define TYPE_METHOD static const HashType Type() { return ClassType(CURRENT_FUNCTION); }
#else
#define TYPE_METHOD static constexpr HashType Type() { return ClassType(CURRENT_FUNCTION); }
#endif

#define OBJECT_TYPEINFO \
public:                 \
TYPE_METHOD

#define COMPONENT_TYPEINFO                                                            \
public:                                                                               \
TYPE_METHOD                                                                           \
static std::string_view Name() { return mName; }                                      \
private:                                                                              \
static consteval std::string_view CacheName() { return ClassName(CURRENT_FUNCTION); } \
static const std::string mName

#define DEFINE_COMPONENT_TYPEINFO(TYPE)                          \
const std::string TYPE::mName = std::string(TYPE::CacheName())

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

template<typename T, typename U>
bool RemoveFromVector(std::vector<T> &vec, const U &value)
{
    return RemoveFromVectorIf(vec, [&value](const T &element) { return element == value; });
}
