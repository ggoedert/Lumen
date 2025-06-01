//==============================================================================================================================================================================
/// \file
/// \brief     engine defines
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_DEFS_H
#define L_DEFS_H

#include <memory>
#include <cstdint>
#include <string>
#include <string_view>

/// Lumen namespace
namespace Lumen
{
    /// typical types
    using byte = uint8_t;
    using word = uint16_t;
    using dword = uint32_t;
    using qword = uint64_t;

    using Hash = dword;
    using Type = Hash;

    /// hash (FNV-1a) range of a string evaluated at compile time
    consteval Hash HashStringRange(const char *string, size_t begin, size_t end)
    {
        Hash hash = static_cast<Hash>(0x811C9DC5);
        while (begin < end)
        {
            hash ^= *(string + begin);
            hash *= static_cast<Hash>(0x01000193);
            begin++;
        }
        return hash;
    }

    /// hash (FNV-1a) string evaluated at compile time
    consteval Hash HashString(const char *string)
    {
        return HashStringRange(string, 0, std::string_view(string).size());
    }

    /// hash (FNV-1a) class name from current function name evaluated at compile time
    consteval Hash HashClassName(const char *currentFunction)
    {
        size_t end = std::string_view(currentFunction).find_last_of('(');
        end = std::string_view(currentFunction, end).find_last_of(':') - 1;
        return HashStringRange(currentFunction, std::string_view(currentFunction, end).find_last_of(' ') + 1, end);
    }

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

#define CLASS_UTILS(TYPE)                                                                                            \
public:                                                                                                              \
using Ptr = std::shared_ptr<TYPE>;                                                                                   \
using WeakPtr = std::weak_ptr<TYPE>;                                                                                 \
using UniquePtr = std::unique_ptr<TYPE>;                                                                             \
template <typename...Args>                                                                                           \
inline static Ptr MakePtr(Args&&...args) { return std::make_shared<TYPE>(std::forward<Args>(args)...); }             \
template <typename...Args>                                                                                           \
inline static UniquePtr MakeUniquePtr(Args&&...args) { return std::make_unique<TYPE>(std::forward<Args>(args)...); } \
static constexpr Lumen::Type Type() { return Lumen::HashClassName(CURRENT_FUNCTION); }                               \
static const std::string &Name() { return mName; }                                                                   \
private:                                                                                                             \
static consteval std::string_view CacheName() { return Lumen::ClassName(CURRENT_FUNCTION); }                         \
static const std::string mName

#endif
