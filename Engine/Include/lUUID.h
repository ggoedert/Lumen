//==============================================================================================================================================================================
/// \file
/// \brief     UUID class for asset management and other unique identifiers, works as key in stl containers
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

#include <random>
#include <charconv>

/// Lumen namespace
namespace Lumen
{
    /// UUID class
    class UUID
    {
        CLASS_NO_DEFAULT_CTOR(UUID);
        friend struct std::hash<UUID>;

    public:
        /// fast static generator using thread-local RNG
        static UUID Generate()
        {
            thread_local std::random_device rd;
            thread_local std::mt19937_64 gen(rd());
            std::uniform_int_distribution<uint64_t> dis;
            UUID id(dis(gen), dis(gen));
            // if defined, create a standard UUID v4, might be usefull for interoperability with external tools
#ifdef MAKE_STANDARD_UUID 
            id.mBytes[6] = (id.mBytes[6] & 0x0F) | 0x40; // version 4
            id.mBytes[8] = (id.mBytes[8] & 0x3F) | 0x80; // variant 1
#endif
            return id;
        }

        /// constructor from 64-bit halves
        explicit UUID(uint64_t low, uint64_t high) : mLow(low), mHigh(high) {}

        /// equality operator
        inline bool operator==(const UUID &other) const
        {
#if defined(SIMDSSE2)
            // compare simd bytes, if all match movemask will be 0xFFFF
            return _mm_movemask_epi8(_mm_cmpeq_epi8(mSimd, other.mSimd)) == 0xFFFF;
#elif defined(SIMDNEON)
            // compare 64-bit blocks
            uint64x2_t vcmp = vceqq_u64(mSimd, other.mSimd);
            return vgetq_lane_u64(vcmp, 0) == ~0ULL && vgetq_lane_u64(vcmp, 1) == ~0ULL;
#else
            // compare 64-bit halves
            return mLow == other.mLow && mHigh == other.mHigh;
#endif
        }

        /// inequality operator
        inline bool operator!=(const UUID &other) const { return !(*this == other); }

        /// less-than operator for std::map / std::set support
        inline bool operator<(const UUID &other) const { return (mHigh != other.mHigh) ? (mHigh < other.mHigh) : (mLow < other.mLow); }

        /// returns a pointer to the raw 16 bytes
        const uint8_t *GetBytes() const { return mBytes; }

        /// sets the raw 16 bytes from a buffer
        void SetBytes(const uint8_t *ptr) { if (ptr) std::memcpy(mBytes, ptr, 16); }

        /// serialize to standard UUID string format
        std::string ToString() const
        {
            char buf[37];
            std::snprintf(buf, sizeof(buf),
                "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                mBytes[0], mBytes[1], mBytes[2], mBytes[3], mBytes[4], mBytes[5], mBytes[6], mBytes[7],
                mBytes[8], mBytes[9], mBytes[10], mBytes[11], mBytes[12], mBytes[13], mBytes[14], mBytes[15]);
            return std::string(buf);
        }

        /// deserialize from standard UUID string format
        static UUID FromString(const std::string &str)
        {
            UUID id(0, 0);
            if (str.length() != 36) return id; // returns invalid zero UUID
            const char *ptr = str.data();
            for (int i = 0; i < 16; ++i)
            {
                if (*ptr == '-') ptr++;
                auto [next, ec] = std::from_chars(ptr, ptr + 2, id.mBytes[i], 16);
                if (ec != std::errc()) return UUID(0, 0);
                ptr = next;
            }
            return id;
        }

        /// check if UUID is valid
        bool Valid() const noexcept { return mHigh != 0 || mLow != 0; }

        /// operator bool for convenience
        explicit operator bool() const noexcept { return Valid(); }

    private:
        /// anonymous union allows access via SIMD vectors, 64-bit halves, or raw mBytes
        union alignas(16)
        {
#if defined(SIMDSSE2)
            __m128i mSimd;
#elif defined(SIMDNEON)
            uint64x2_t mSimd;
#endif
            struct
            {
                uint64_t mLow;
                uint64_t mHigh;
            };
            uint8_t mBytes[16];
        };
    };
}

/// hash support in std namespace for stl containers like unordered_map
namespace std
{
    template <>
    struct hash<Lumen::UUID>
    {
        size_t operator()(const Lumen::UUID &uuid) const
        {
            // because the UUID is pseudo-random, an XOR of the halves creates an fast and collision-resistant hash
            return static_cast<size_t>(uuid.mHigh ^ uuid.mLow);
        }
    };
}
