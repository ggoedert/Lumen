//==============================================================================================================================================================================
/// \file
/// \brief     Math
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

// platform specific includes
#ifdef _WIN32
#include <sal.h>
#else
#define _In_reads_(s)
#endif
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
#define SIMDSSE
#include <xmmintrin.h>
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#define SIMDNEON
#include <arm_neon.h>
#else
#define SIMDSCALAR
#endif

/// Lumen Math namespace
namespace Lumen::Math
{
    /// SIMD Vector
#if defined(SIMDSSE)
    using SIMDVECTOR = __m128;
#elif defined(SIMDNEON)
    using SIMDVECTOR = float32x4_t;
#else
    struct SIMDVECTOR
    {
        union
        {
            float    mFVector4[4];
            uint32_t mUIVector4[4];
        };
    };
#endif

    /// 3D vector with float components
    struct Float3
    {
        Float3() = default;

        Float3(const Float3 &) = default;
        Float3 &operator=(const Float3 &) = default;

        Float3(Float3 &&) = default;
        Float3 &operator=(Float3 &&) = default;

        constexpr Float3(float _x, float _y, float _z) noexcept : x(_x), y(_y), z(_z) {}
        explicit Float3(_In_reads_(3) const float *pArray) noexcept : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}

        float x, y, z;
    };

    /// SIMD Vector with 16-byte alignment
    struct alignas(16) Vector
    {
        inline operator SIMDVECTOR() const noexcept { return mV; }
        inline operator const float *() const noexcept { return mF; }

        union
        {
            float mF[4];
            SIMDVECTOR mV;
        };
    };

    /// 3D vector with float components, derived from Float3, with extra functionality
    struct Vector3 : public Float3
    {
        Vector3() noexcept : Float3(0.f, 0.f, 0.f) {}
        constexpr explicit Vector3(float ix) noexcept : Float3(ix, ix, ix) {}
        constexpr Vector3(float ix, float iy, float iz) noexcept : Float3(ix, iy, iz) {}
        explicit Vector3(_In_reads_(3) const float *pArray) noexcept : Float3(pArray) {}
        Vector3(const Float3 &V) noexcept { this->x = V.x; this->y = V.y; this->z = V.z; }

        Vector3(const Vector3 &) = default;
        Vector3 &operator=(const Vector3 &) = default;

        Vector3(Vector3 &&) = default;
        Vector3 &operator=(Vector3 &&) = default;
    };
}
