//==============================================================================================================================================================================
/// \file
/// \brief     engine defines
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDebugLog.h"

/// \cond
#include <array>
#include <algorithm>
/// \endcond

/// sse platform specific includes
#ifdef _WIN32
#include <sal.h>
#else
#define _In_reads_(s)
#endif
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define SIMDSSE2
#include <emmintrin.h>
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#define SIMDNEON
#include <arm_neon.h>
#else
#define SIMDSCALAR
#endif

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

    /// return the shared Base64 table for encoding function, hopefully optimized away by the compiler if not used
    consteval const char *GetBase64Table() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; }

    /// encode bytes into a Base64 string
    inline std::string Base64EncodeBytes(const std::vector<byte> &bytes)
    {
        if (bytes.empty()) return "";

        const byte *data = bytes.data();
        const size_t len = bytes.size();
        const char *table = GetBase64Table();
        std::string out;
        out.reserve((len + 2) / 3 * 4);

        for (size_t i = 0; i < len; i += 3)
        {
            // pack 3 bytes into a 24-bit integer
            dword val24 = data[i];
            val24 = (val24 << 8) | ((i + 1 < len) ? data[i + 1] : 0);
            val24 = (val24 << 8) | ((i + 2 < len) ? data[i + 2] : 0);

            // extract 4 sets of 6-bits and map to Base64
            out.push_back(table[(val24 >> 18) & 0x3F]);
            out.push_back(table[(val24 >> 12) & 0x3F]);
            if (i + 1 < len) out.push_back(table[(val24 >> 6) & 0x3F]);
            if (i + 2 < len) out.push_back(table[val24 & 0x3F]);
        }

        // return encoded string
        return out;
    }

    /// decode Base64 string into bytes
    inline std::vector<byte> Base64DecodeBytes(const std::string &input)
    {
        if (input.empty()) return {};

        dword val = 0;
        int bits = 0;
        std::vector<byte> out;
        out.reserve(input.length() * 3 / 4);

        // initialize reversed table once
        static const std::array<byte, 256> reversedTable = []()
        {
            const char *table = GetBase64Table();
            std::array<byte, 256> returnTable;
            returnTable.fill(0xFF); // 0xFF marks invalid chars
            for (int i = 0; i < 64; ++i)
            {
                returnTable[static_cast<unsigned char>(table[i])] = i;
            }
            return returnTable;
        }();

        // decode input characters
        for (char c : input)
        {
            byte decoded = reversedTable[static_cast<unsigned char>(c)];
            if (decoded == 0xFF) continue;

            val = (val << 6) | decoded;
            bits += 6;

            if (bits >= 8)
            {
                bits -= 8;
                out.push_back((val >> bits) & 0xFF);
            }
        }

        // return decoded bytes
        return out;
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
