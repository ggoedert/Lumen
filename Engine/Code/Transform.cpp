//==============================================================================================================================================================================
/// \file
/// \brief     Transform
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lTransform.h"

using namespace Lumen;

/// Transform::Impl class
class Transform::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs a transform
    explicit Impl(const EntityWeakPtr &entity) : mEntity(entity) {}

    /// destroys transform
    ~Impl() = default;

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        // set position
        if (mPosition != Math::Vector3::cZero)
        {
            Serialized::SerializeValue(out, packed, Serialized::cPositionToken, Serialized::cPositionTokenPacked,
                std::array { mPosition.x, mPosition.y, mPosition.z });
        }

        // set rotation
        if (mRotation != Math::Quaternion::cIdentity)
        {
            Serialized::SerializeValue(out, packed, Serialized::cRotationToken, Serialized::cRotationTokenPacked,
                std::array { mRotation.x, mRotation.y, mRotation.z, mRotation.w });
        }

        // set scale
        if (mScale != Math::Vector3::cOne)
        {
            Serialized::SerializeValue(out, packed, Serialized::cScaleToken, Serialized::cScaleTokenPacked,
                std::array { mScale.x, mScale.y, mScale.z });
        }
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        Serialized::Type value;

        // get position
        mPosition = Math::Vector3::cZero;
        if (Serialized::DeserializeValue(in, packed, Serialized::cPositionToken, Serialized::cPositionTokenPacked, value))
        {
            if (value.size() != 3)
            {
                throw std::runtime_error(std::format("Unable to read Transform::Position"));
            }
            mPosition = Math::Vector3 { value.get<std::vector<float>>().data() };
        }

        // get rotation
        mRotation = Math::Quaternion::cIdentity;
        if (Serialized::DeserializeValue(in, packed, Serialized::cRotationToken, Serialized::cRotationTokenPacked, value))
        {
            if (value.size() != 4)
            {
                throw std::runtime_error(std::format("Unable to read Transform::Rotation"));
            }
            mRotation = Math::Vector4 { value.get<std::vector<float>>().data() };
        }

        // get scale
        mScale = Math::Vector3::cOne;
        if (Serialized::DeserializeValue(in, packed, Serialized::cScaleToken, Serialized::cScaleTokenPacked, value))
        {
            if (value.size() != 3)
            {
                throw std::runtime_error(std::format("Unable to read Transform::Scale"));
            }
            mPosition = Math::Vector3 { value.get<std::vector<float>>().data() };
        }
    }

    /// get owning entity
    [[nodiscard]] EntityWeakPtr Entity() const { return mEntity; }

    /// get parent
    [[nodiscard]] const TransformWeakPtr &GetParent() const { return mParent; }

    /// set parent
    void SetParent(const TransformWeakPtr &parent)
    {
        L_ASSERT(parent.lock() != mOwner.lock());
        mParent = parent;
    }

    /// get position
    [[nodiscard]] const Math::Vector3 &GetPosition() const { return mPosition; }

    /// set position
    void SetPosition(const Math::Vector3 &position) { mPosition = position; }

    /// translate by x, y, z
    void Translate(float x, float y, float z)
    {
        mPosition.x += x;
        mPosition.y += y;
        mPosition.z += z;
    }

    /// get rotation
    [[nodiscard]] const Math::Quaternion &GetRotation() const { return mRotation; }

    /// set rotation
    void SetRotation(const Math::Quaternion &rotation) { mRotation = rotation; }

    /// rotate by euler angles in degrees
    void Rotate(float xAngle, float yAngle, float zAngle)
    {
        // apply rotation incrementally
        mRotation = mRotation * Math::Quaternion::FromYawPitchRoll(yAngle, xAngle, zAngle);

        // normalize to avoid drift over time
        mRotation.Normalize();
    }

    /// get world matrix
    void GetWorldMatrix(Math::Matrix44 &world) const
    {
        world = Math::Matrix44::cIdentity;
        if (auto parentLock = mParent.lock())
        {
            Math::Matrix44 parentWorld;
            parentLock->mImpl->GetWorldMatrix(parentWorld);
            world = parentWorld;
        }
        Math::Float44 translation = Math::Matrix44::Translation(mPosition);
        Math::Float44 rotation = Math::Matrix44::FromQuaternion(mRotation);
        Math::Float44 scale = Math::Matrix44::Scale(mScale);
        //world = world * translation * rotation * scale;
        world = world * scale * rotation * translation;
    }

    /// owner
    TransformWeakPtr mOwner;

    /// owning entity
    EntityWeakPtr mEntity;

    /// parent transform
    TransformWeakPtr mParent;

    /// position
    Math::Vector3 mPosition;

    /// rotation
    Math::Quaternion mRotation;

    /// scale
    Math::Vector3 mScale = Math::Vector3::cOne;
};

//==============================================================================================================================================================================

/// constructor
Transform::Transform(const EntityWeakPtr &entity) : Object(Type()), mImpl(Transform::Impl::MakeUniquePtr(entity)) {}

/// destructor
Transform::~Transform() = default;

/// creates a smart pointer version of the transform component
TransformPtr Transform::MakePtr(const EntityWeakPtr &entity)
{
    auto ptr = TransformPtr(new Transform(entity));
    ptr->mImpl->mOwner = ptr;
    return ptr;
}

/// serialize
void Transform::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Transform::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// get owning entity
EntityWeakPtr Transform::Entity() const
{
    return mImpl->Entity();
}

/// get parent
const TransformWeakPtr &Transform::GetParent() const
{
    return mImpl->GetParent();
}

/// set parent
void Transform::SetParent(const TransformWeakPtr &parent)
{
    return mImpl->SetParent(parent);
}

/// get position
const Math::Vector3 &Transform::GetPosition() const
{
    return mImpl->GetPosition();
}

/// set position
void Transform::SetPosition(const Math::Vector3 &position)
{
    return mImpl->SetPosition(position);
}

/// translate by x, y, z
void Transform::Translate(float x, float y, float z)
{
    mImpl->Translate(x, y, z);
}

/// get rotation
[[nodiscard]] const Math::Quaternion &Transform::GetRotation() const
{
    return mImpl->GetRotation();
}

/// set rotation
void Transform::SetRotation(const Math::Quaternion &rotation)
{
    return mImpl->SetRotation(rotation);
}

/// rotate by euler angles in degrees
void Transform::Rotate(float xAngle, float yAngle, float zAngle)
{
    mImpl->Rotate(xAngle, yAngle, zAngle);
}

/// get world matrix
void Transform::GetWorldMatrix(Math::Matrix44 &world) const
{
    return mImpl->GetWorldMatrix(world);
}
