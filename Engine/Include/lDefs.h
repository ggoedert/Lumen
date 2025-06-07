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
#include <any>

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

#ifdef _DEBUG
    /// debug version of Type
    struct Type
    {
        Type(Hash hash, std::string_view label) : mHash(hash), mLabel(label) {}
        operator Hash() const { return mHash; }
        bool operator==(const Type &other) const { return mHash == other.mHash; }
        Hash mHash;
        std::string_view mLabel;
    };

    /// hash (FNV-1a) class name from current function name, debug version
    Type ClassNameType(const char *currentFunction);
#else
    /// lean version of Type
    using Type = Hash;

    /// hash (FNV-1a) class name from current function name evaluated at compile time
    consteval Type ClassNameType(const char *currentFunction)
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

/// current enclosing function
#if defined(__INTELLISENSE__)
#define CURRENT_FUNCTION "Type Class::Method()"
#elif defined(__FUNCSIG__)
#define CURRENT_FUNCTION __FUNCSIG__
#elif defined(__PRETTY_FUNCTION__)
#define CURRENT_FUNCTION __PRETTY_FUNCTION__
#else
#error Unable to determine the current function.
#endif

#define CLASS_NO_DEFAULT_CTOR(TYPE) \
public:                             \
TYPE() = delete

#define CLASS_NO_COPY_MOVE(TYPE)        \
public:                                 \
TYPE(const TYPE &) = delete;            \
TYPE &operator=(const TYPE &) = delete; \
TYPE(TYPE &&) = delete;                 \
TYPE &operator=(TYPE &&) = delete

#define CLASS_PTR_DEFS(TYPE)                   \
class TYPE;                                    \
using TYPE##Ptr = std::shared_ptr<TYPE>;       \
using TYPE##WeakPtr = std::weak_ptr<TYPE>;     \
using TYPE##UniquePtr = std::unique_ptr<TYPE>

#define CLASS_PTR_MAKERS(TYPE)                                                                                            \
public:                                                                                                                   \
template <typename...Args>                                                                                                \
inline static TYPE##Ptr MakePtr(Args&&...args) { return std::make_shared<TYPE>(std::forward<Args>(args)...); }            \
template <typename...Args>                                                                                                \
inline static TYPE##UniquePtr MakeUniquePtr(Args&&...args) { return std::make_unique<TYPE>(std::forward<Args>(args)...); }

#ifdef _DEBUG
#define COMPONENTTYPE_METHOD static const Lumen::Type ComponentType() { return Lumen::ClassNameType(CURRENT_FUNCTION); }
#else
#define COMPONENTTYPE_METHOD static constexpr Lumen::Type ComponentType() { return Lumen::ClassNameType(CURRENT_FUNCTION); }
#endif

#define COMPONENT_TRAITS(TYPE)                                                                        \
public:                                                                                               \
COMPONENTTYPE_METHOD                                                                                  \
static const std::string &ComponentName() { return mComponentName; }                                  \
private:                                                                                              \
static consteval std::string_view CacheComponentName() { return Lumen::ClassName(CURRENT_FUNCTION); } \
static const std::string mComponentName

#define DEFINE_COMPONENT_TRAITS(TYPE)                                             \
const std::string TYPE::mComponentName = std::string(TYPE::CacheComponentName())

#define COMPONENT_UTILS(TYPE) \
CLASS_PTR_MAKERS(TYPE)        \
COMPONENT_TRAITS(TYPE)
