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
    explicit Impl(const GameObjectWeakPtr &gameObject) : mGameObject(gameObject) {}

    /// destroys transform
    ~Impl() = default;

    /// serialize
    void Serialize(json &out) const
    {
        //mImpl->Serialize(out);
    }

    /// deserialize
    void Deserialize(const json &in)
    {
        // set position
        mPosition = {};
        if (in.contains("Position"))
        {
            auto arr = in["Position"].get<std::vector<float>>();
            if (arr.size() == 3)
            {
                mPosition = Math::Vector3(arr[0], arr[1], arr[2]);
            }
        }

        // set rotation
        mRotation = {};
        if (in.contains("Rotation"))
        {
            auto arr = in["Rotation"].get<std::vector<float>>();
            if (arr.size() == 4)
            {
                mRotation = Math::Vector4(arr[0], arr[1], arr[2], arr[3]);
            }
        }

        // set scale
        mScale = Math::Vector3::cOne;
        if (in.contains("Scale"))
        {
            auto arr = in["Scale"].get<std::vector<float>>();
            if (arr.size() == 3)
            {
                mScale = Math::Vector3(arr[0], arr[1], arr[2]);
            }
        }
    }

    /// get owning game object
    [[nodiscard]] GameObjectWeakPtr GameObject() const { return mGameObject; }

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

    /// owning game object
    GameObjectWeakPtr mGameObject;

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
Transform::Transform(const GameObjectWeakPtr &gameObject) : Object(Type()), mImpl(Transform::Impl::MakeUniquePtr(gameObject)) {}

/// destructor
Transform::~Transform() = default;

/// creates a smart pointer version of the transform component
TransformPtr Transform::MakePtr(const GameObjectWeakPtr &gameObject)
{
    auto pTransform = new Transform(gameObject);
    auto transformPtr = TransformPtr(pTransform);
    pTransform->mImpl->mOwner = transformPtr;
    return transformPtr;
}

/// serialize
void Transform::Serialize(json &out) const
{
    mImpl->Serialize(out);
}

/// deserialize
void Transform::Deserialize(const json &in)
{
    mImpl->Deserialize(in);
}

/// get owning game object
GameObjectWeakPtr Transform::GameObject() const
{
    return mImpl->GameObject();
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
