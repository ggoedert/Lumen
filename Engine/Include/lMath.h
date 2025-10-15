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

#include <cstring>

/// Lumen Math namespace
namespace Lumen::Math
{
    struct Float2;
    struct Float3;
    struct Float4;
    struct Vector2;
    struct Vector3;
    struct Vector4;
    struct Matrix22;
    struct Matrix33;
    struct Matrix44;
    struct Quaternion;

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

    constexpr float PI = 3.14159265358979323846f;
    constexpr float ToRadians(float degrees) noexcept { return degrees * (PI / 180.f); }
    constexpr float ToDegrees(float radians) noexcept { return radians * (180.f / PI); }

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

    /// 2D vector with float components
    struct Float2
    {
        Float2() = default;

        Float2(const Float2 &) = default;
        Float2 &operator=(const Float2 &) = default;

        Float2(Float2 &&) = default;
        Float2 &operator=(Float2 &&) = default;

        constexpr Float2(float _x, float _y) noexcept : x(_x), y(_y) {}
        explicit Float2(_In_reads_(2) const float *pArray) noexcept : x(pArray[0]), y(pArray[1]) {}

        float x, y;
    };

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

    /// 4D vector with float components
    struct Float4
    {
        Float4() = default;

        Float4(const Float4 &) = default;
        Float4 &operator=(const Float4 &) = default;

        Float4(Float4 &&) = default;
        Float4 &operator=(Float4 &&) = default;

        constexpr Float4(float _x, float _y, float _z, float _w) noexcept : x(_x), y(_y), z(_z), w(_w) {}
        explicit Float4(_In_reads_(4) const float *pArray) noexcept : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}

        float x, y, z, w;
    };

    /// 2x2 Matrix: 32 bit floating point components
    struct Float22
    {
        Float22() = default;

        Float22(const Float22 &) = default;
        Float22 &operator=(const Float22 &) = default;

        Float22(Float22 &&) = default;
        Float22 &operator=(Float22 &&) = default;

        constexpr Float22(float m00, float m01,
                          float m10, float m11) noexcept :
            _11(m00), _12(m01),
            _21(m10), _22(m11) {}
        explicit Float22(_In_reads_(4) const float *pArray) noexcept;

        float operator() (size_t r, size_t c) const noexcept { return m[r][c]; }
        float &operator() (size_t r, size_t c) noexcept { return m[r][c]; }

        const float *operator*() const noexcept { return &_11; }

        union
        {
            struct
            {
                float _11, _12;
                float _21, _22;
            };
            float m[2][2];
        };
    };

    /// 3x3 Matrix: 32 bit floating point components
    struct Float33
    {
        Float33() = default;

        Float33(const Float33 &) = default;
        Float33 &operator=(const Float33 &) = default;

        Float33(Float33 &&) = default;
        Float33 &operator=(Float33 &&) = default;

        constexpr Float33(float m00, float m01, float m02,
                          float m10, float m11, float m12,
                          float m20, float m21, float m22) noexcept :
            _11(m00), _12(m01), _13(m02),
            _21(m10), _22(m11), _23(m12),
            _31(m20), _32(m21), _33(m22) {}
        explicit Float33(_In_reads_(9) const float *pArray) noexcept;

        float operator() (size_t r, size_t c) const noexcept { return m[r][c]; }
        float &operator() (size_t r, size_t c) noexcept { return m[r][c]; }

        const float *operator*() const noexcept { return &_11; }

        union
        {
            struct
            {
                float _11, _12, _13;
                float _21, _22, _23;
                float _31, _32, _33;
            };
            float m[3][3];
        };
    };

    /// 4x4 Matrix: 32 bit floating point components
    struct Float44
    {
        Float44() = default;

        Float44(const Float44 &) = default;
        Float44 &operator=(const Float44 &) = default;

        Float44(Float44 &&) = default;
        Float44 &operator=(Float44 &&) = default;

        constexpr Float44(float m00, float m01, float m02, float m03,
                          float m10, float m11, float m12, float m13,
                          float m20, float m21, float m22, float m23,
                          float m30, float m31, float m32, float m33) noexcept :
            _11(m00), _12(m01), _13(m02), _14(m03),
            _21(m10), _22(m11), _23(m12), _24(m13),
            _31(m20), _32(m21), _33(m22), _34(m23),
            _41(m30), _42(m31), _43(m32), _44(m33) {}
        explicit Float44(_In_reads_(16) const float *pArray) noexcept;

        float operator() (size_t r, size_t c) const noexcept { return m[r][c]; }
        float &operator() (size_t r, size_t c) noexcept { return m[r][c]; }

        const float *operator*() const noexcept { return &_11; }

        union
        {
            struct
            {
                float _11, _12, _13, _14;
                float _21, _22, _23, _24;
                float _31, _32, _33, _34;
                float _41, _42, _43, _44;
            };
            float m[4][4];
        };
    };

    /// 2D vector with float components, derived from Float2, with extra functionality
    struct Vector2 : public Float2
    {
        Vector2() noexcept : Float2(0.f, 0.f) {}
        constexpr explicit Vector2(float x) noexcept : Float2(x, x) {}
        constexpr Vector2(float x, float y, float z) noexcept : Float2(x, y) {}
        explicit Vector2(_In_reads_(2) const float *pArray) noexcept : Float2(pArray) {}
        Vector2(const Float2 &f) noexcept { this->x = f.x; this->y = f.y; }
        explicit Vector2(const Vector &v) noexcept { this->x = v.mF[0]; this->y = v.mF[1]; }

        Vector2(const Vector2 &) = default;
        Vector2 &operator=(const Vector2 &) = default;

        Vector2(Vector2 &&) = default;
        Vector2 &operator=(Vector2 &&) = default;

        operator Vector() const noexcept { return Vector { this->x, this->y, 0.f, 0.f }; }

        static const Vector2 cZero;
        static const Vector2 cOne;
    };

    /// 3D vector with float components, derived from Float3, with extra functionality
    struct Vector3 : public Float3
    {
        Vector3() noexcept : Float3(0.f, 0.f, 0.f) {}
        constexpr explicit Vector3(float x) noexcept : Float3(x, x, x) {}
        constexpr Vector3(float x, float y, float z) noexcept : Float3(x, y, z) {}
        explicit Vector3(_In_reads_(3) const float *pArray) noexcept : Float3(pArray) {}
        Vector3(const Float3 &f) noexcept { this->x = f.x; this->y = f.y; this->z = f.z; }
        explicit Vector3(const Vector &v) noexcept { this->x = v.mF[0]; this->y = v.mF[1]; this->z = v.mF[2]; }

        Vector3(const Vector3 &) = default;
        Vector3 &operator=(const Vector3 &) = default;

        Vector3(Vector3 &&) = default;
        Vector3 &operator=(Vector3 &&) = default;

        operator Vector() const noexcept { return Vector { this->x, this->y, this->z, 0.f }; }

        static const Vector3 cZero;
        static const Vector3 cOne;
    };

    /// 4D vector with float components, derived from Float4, with extra functionality
    struct Vector4 : public Float4
    {
        Vector4() noexcept : Float4(0.f, 0.f, 0.f, 0.f) {}
        constexpr explicit Vector4(float x) noexcept : Float4(x, x, x, x) {}
        constexpr Vector4(float x, float y, float z, float w) noexcept : Float4(x, y, z, w) {}
        explicit Vector4(_In_reads_(4) const float *pArray) noexcept : Float4(pArray) {}
        Vector4(const Float4 &f) noexcept { this->x = f.x; this->y = f.y; this->z = f.z; this->w = f.w; }
        explicit Vector4(const Vector &v) noexcept { this->x = v.mF[0]; this->y = v.mF[1]; this->z = v.mF[2]; this->w = v.mF[3]; }

        Vector4(const Vector4 &) = default;
        Vector4 &operator=(const Vector4 &) = default;

        Vector4(Vector4 &&) = default;
        Vector4 &operator=(Vector4 &&) = default;

        operator Vector() const noexcept { return Vector { this->x, this->y, this->z, this->w }; }

        static const Vector4 cZero;
        static const Vector4 cOne;
    };

    /// 2x2 Matrix (assumes right-handed cooordinates)
    struct Matrix22 : public Float22
    {
        Matrix22() noexcept :
            Float22(1.f, 0.f,
                    0.f, 1.f) {}
        constexpr Matrix22(float m00, float m01,
                           float m10, float m11) noexcept :
            Float22(m00, m01,
                    m10, m11) {}
        explicit Matrix22(const Vector2 &r0, const Vector2 &r1) noexcept :
            Float22(r0.x, r0.y,
                    r1.x, r1.y) {}
        Matrix22(const Float22 &m) noexcept { memcpy(this, &m, sizeof(Float22)); }

        /// constants
        static const Matrix22 cIdentity;
    };

    /// 3x3 Matrix (assumes right-handed cooordinates)
    struct Matrix33 : public Float33
    {
        Matrix33() noexcept :
            Float33(1.f, 0.f, 0.f,
                    0.f, 1.f, 0.f,
                    0.f, 0.f, 1.f) {}
        constexpr Matrix33(float m00, float m01, float m02,
                           float m10, float m11, float m12,
                           float m20, float m21, float m22) noexcept :
            Float33(m00, m01, m02,
                    m10, m11, m12,
                    m20, m21, m22) {}
        explicit Matrix33(const Vector3 &r0, const Vector3 &r1, const Vector3 &r2, const Vector3 &r3) noexcept :
            Float33(r0.x, r0.y, r0.z,
                    r1.x, r1.y, r1.z,
                    r2.x, r2.y, r2.z) {}
        Matrix33(const Float33 &m) noexcept { memcpy(this, &m, sizeof(Float33)); }

        /// constants
        static const Matrix33 cIdentity;
    };

    /// 4x4 Matrix (assumes right-handed cooordinates)
    struct Matrix44 : public Float44
    {
        Matrix44() noexcept :
            Float44(1.f, 0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f) {}
        constexpr Matrix44(float m00, float m01, float m02, float m03,
                           float m10, float m11, float m12, float m13,
                           float m20, float m21, float m22, float m23,
                           float m30, float m31, float m32, float m33) noexcept :
            Float44(m00, m01, m02, m03,
                    m10, m11, m12, m13,
                    m20, m21, m22, m23,
                    m30, m31, m32, m33) {}
        explicit Matrix44(const Vector3 &r0, const Vector3 &r1, const Vector3 &r2) noexcept :
            Float44(r0.x, r0.y, r0.z, 0.f,
                    r1.x, r1.y, r1.z, 0.f,
                    r2.x, r2.y, r2.z, 0.f,
                    0.f,  0.f,  0.f,  1.f) {}
        explicit Matrix44(const Vector4 &r0, const Vector4 &r1, const Vector4 &r2, const Vector4 &r3) noexcept :
            Float44(r0.x, r0.y, r0.z, r0.w,
                    r1.x, r1.y, r1.z, r1.w,
                    r2.x, r2.y, r2.z, r2.w,
                    r3.x, r3.y, r3.z, r3.w) {}
        Matrix44(const Float44 &m) noexcept { memcpy(this, &m, sizeof(Float44)); }
        Matrix44(const Float33 &m) noexcept;


        /// assignment operators
        Matrix44 &operator+=(const Matrix44 &m) noexcept;
        Matrix44 &operator-=(const Matrix44 &m) noexcept;
        Matrix44 &operator*=(const Matrix44 &m) noexcept;
        Matrix44 &operator*=(float s) noexcept;
        Matrix44 &operator/=(float s) noexcept;

        /// static functions
        static Matrix44 Translation(const Vector3 &position) noexcept;
        static Matrix44 Translation(float x, float y, float z) noexcept;

        static Matrix44 Scale(const Vector3 &scale) noexcept;
        static Matrix44 Scale(float xs, float ys, float zs) noexcept;
        static Matrix44 Scale(float scale) noexcept;

        static Matrix44 RotationX(float radian) noexcept;
        static Matrix44 RotationY(float radian) noexcept;
        static Matrix44 RotationZ(float radian) noexcept;

        static Matrix44 FromAxisAngle(const Vector3 &axis, float angle) noexcept;

        static Matrix44 PerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept;
        static Matrix44 Perspective(float width, float height, float nearPlane, float farPlane) noexcept;
        static Matrix44 PerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept;
        static Matrix44 Orthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept;
        static Matrix44 OrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

        static Matrix44 LookAt(const Vector3 &position, const Vector3 &target, const Vector3 &up) noexcept;
        static Matrix44 World(const Vector3 &position, const Vector3 &forward, const Vector3 &up) noexcept;

        static Matrix44 FromQuaternion(const Quaternion &quat) noexcept;

        /// constants
        static const Matrix44 cIdentity;
    };

    /// binary operators
    Matrix44 operator+(const Matrix44 &m1, const Matrix44 &m2) noexcept;
    Matrix44 operator-(const Matrix44 &m1, const Matrix44 &m2) noexcept;
    Matrix44 operator*(const Matrix44 &m1, const Matrix44 &m2) noexcept;
    Matrix44 operator*(const Matrix44 &m, float s) noexcept;
    Matrix44 operator*(float s, const Matrix44 &m) noexcept;
    Matrix44 operator/(const Matrix44 &m, float s) noexcept;
    Matrix44 operator/(const Matrix44 &m1, const Matrix44 &m2) noexcept;

    /// element-wise divide
    Matrix44 operator/(float s, const Matrix44 &m) noexcept;

    struct Quaternion : public Float4
    {
        Quaternion() noexcept : Float4(0.f, 0.f, 0.f, 1.f) {}
        constexpr Quaternion(float x, float y, float z, float w) noexcept : Float4(x, y, z, w) {}
        Quaternion(const Vector3 &v, float scalar) noexcept : Float4(v.x, v.y, v.z, scalar) {}
        explicit Quaternion(const Vector4 &v) noexcept : Float4(v.x, v.y, v.z, v.w) {}
        explicit Quaternion(_In_reads_(4) const float *pArray) noexcept : Float4(pArray) {}
        Quaternion(const Float4 &q) noexcept { this->x = q.x; this->y = q.y; this->z = q.z; this->w = q.w; }
        explicit Quaternion(const Vector &F) noexcept { this->x = F.mF[0]; this->y = F.mF[1]; this->z = F.mF[2]; this->w = F.mF[3]; }

        Quaternion(const Quaternion &) = default;
        Quaternion &operator=(const Quaternion &) = default;

        Quaternion(Quaternion &&) = default;
        Quaternion &operator=(Quaternion &&) = default;

        operator Vector() const noexcept { return Vector { this->x, this->y, this->z, this->w }; }

        // Static functions
        static Quaternion FromAxisAngle(const Vector3 &axis, float angle) noexcept;

        // Rotates about y-axis (yaw), then x-axis (pitch), then z-axis (roll)
        static Quaternion FromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

        // Rotates about y-axis (angles.y), then x-axis (angles.x), then z-axis (angles.z)
        static Quaternion FromYawPitchRoll(const Vector3 &angles) noexcept;

        static Quaternion FromRotationMatrix(const Matrix44 &M) noexcept;

        // Quaternion operations
        float Length() const noexcept;

        void Normalize() noexcept;

        void Conjugate() noexcept;

        void Inverse(Quaternion &result) const noexcept;

        float Dot(const Quaternion &Q) const noexcept;

        void RotateTowards(const Quaternion &target, float maxAngle) noexcept;

        /// constants
        static const Quaternion cIdentity;
    };

    /// binary operators
    Quaternion operator+(const Quaternion &q1, const Quaternion &q2) noexcept;
    Quaternion operator-(const Quaternion &q1, const Quaternion &q2) noexcept;
    Quaternion operator*(const Quaternion &q1, const Quaternion &q2) noexcept;
    Quaternion operator*(const Quaternion &q, float s) noexcept;
    Quaternion operator*(float s, const Quaternion &q) noexcept;
    Quaternion operator/(const Quaternion &q1, const Quaternion &q2) noexcept;
}
