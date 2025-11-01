//==============================================================================================================================================================================
/// \file
/// \brief     Math
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMath.h"

#include <cmath>

using namespace Lumen;

const Math::Vector2 Math::Vector2::cZero { 0.f, 0.f };
const Math::Vector2 Math::Vector2::cOne { 1.f, 1.f };
const Math::Vector3 Math::Vector3::cZero { 0.f, 0.f, 0.f };
const Math::Vector3 Math::Vector3::cOne { 1.f, 1.f, 1.f };
const Math::Vector4 Math::Vector4::cZero { 0.f, 0.f, 0.f, 0.f };
const Math::Vector4 Math::Vector4::cOne { 1.f, 1.f, 1.f, 1.f };

const Math::Matrix22 Math::Matrix22::cIdentity { 1.f, 0.f,
                                                 0.f, 1.f };
const Math::Matrix33 Math::Matrix33::cIdentity { 1.f, 0.f, 0.f,
                                                 0.f, 1.f, 0.f,
                                                 0.f, 0.f, 1.f };
const Math::Matrix44 Math::Matrix44::cIdentity { 1.f, 0.f, 0.f, 0.f,
                                                 0.f, 1.f, 0.f, 0.f,
                                                 0.f, 0.f, 1.f, 0.f,
                                                 0.f, 0.f, 0.f, 1.f };

const Math::Quaternion Math::Quaternion::cIdentity { 0.f, 0.f, 0.f, 1.f };

// Translation matrix
Math::Matrix44 Math::Matrix44::Translation(const Math::Vector3 &position) noexcept
{
    Matrix44 result;
    result._11 = 1.f; result._12 = 0.f; result._13 = 0.f; result._14 = 0.f;
    result._21 = 0.f; result._22 = 1.f; result._23 = 0.f; result._24 = 0.f;
    result._31 = 0.f; result._32 = 0.f; result._33 = 1.f; result._34 = 0.f;
    result._41 = position.x; result._42 = position.y; result._43 = position.z; result._44 = 1.f;
    return result;
}

// Scale matrix
Math::Matrix44 Math::Matrix44::Scale(const Math::Vector3 &scale) noexcept
{
    Matrix44 result;
    result._11 = scale.x; result._12 = 0.f;     result._13 = 0.f;     result._14 = 0.f;
    result._21 = 0.f;     result._22 = scale.y; result._23 = 0.f;     result._24 = 0.f;
    result._31 = 0.f;     result._32 = 0.f;     result._33 = scale.z; result._34 = 0.f;
    result._41 = 0.f;     result._42 = 0.f;     result._43 = 0.f;     result._44 = 1.f;
    return result;
}

// From quaternion
Math::Matrix44 Math::Matrix44::FromQuaternion(const Math::Quaternion &quat) noexcept
{
    Matrix44 result;

    const float xx = quat.x * quat.x;
    const float yy = quat.y * quat.y;
    const float zz = quat.z * quat.z;
    const float xy = quat.x * quat.y;
    const float xz = quat.x * quat.z;
    const float yz = quat.y * quat.z;
    const float wx = quat.w * quat.x;
    const float wy = quat.w * quat.y;
    const float wz = quat.w * quat.z;

    result._11 = 1.f - 2.f * (yy + zz);
    result._12 = 2.f * (xy + wz);
    result._13 = 2.f * (xz - wy);
    result._14 = 0.f;

    result._21 = 2.f * (xy - wz);
    result._22 = 1.f - 2.f * (xx + zz);
    result._23 = 2.f * (yz + wx);
    result._24 = 0.f;

    result._31 = 2.f * (xz + wy);
    result._32 = 2.f * (yz - wx);
    result._33 = 1.f - 2.f * (xx + yy);
    result._34 = 0.f;

    result._41 = 0.f;
    result._42 = 0.f;
    result._43 = 0.f;
    result._44 = 1.f;

    return result;
}

// Matrix multiplication
Math::Matrix44 Math::operator*(const Math::Matrix44 &m1, const Math::Matrix44 &m2) noexcept
{
    Matrix44 result = {};

    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            result.m[row][col] =
                m1.m[row][0] * m2.m[0][col] +
                m1.m[row][1] * m2.m[1][col] +
                m1.m[row][2] * m2.m[2][col] +
                m1.m[row][3] * m2.m[3][col];
        }
    }

    return result;
}

Math::Quaternion Math::Quaternion::FromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
    // Half angles
    float cy = std::cos(ToRadians(yaw * 0.5f));
    float sy = std::sin(ToRadians(yaw * 0.5f));
    float cp = std::cos(ToRadians(pitch * 0.5f));
    float sp = std::sin(ToRadians(pitch * 0.5f));
    float cr = std::cos(ToRadians(roll * 0.5f));
    float sr = std::sin(ToRadians(roll * 0.5f));

    Math::Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

float Math::Quaternion::Length() const noexcept
{
    return std::sqrt(x * x + y * y + z * z + w * w);
}

void Math::Quaternion::Normalize() noexcept
{
    const float len = Length();
    if (len > 0.f)
    {
        const float invLen = 1.f / len;
        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;
    }
    else
    {
        // zero length quaternion, set to identity
        x = 0.f;
        y = 0.f;
        z = 0.f;
        w = 1.f;
    }
}

Math::Quaternion Math::operator*(const Math::Quaternion &q1, const Math::Quaternion &q2) noexcept
{
    Math::Quaternion result;
    result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    return result;
}
