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
        world = world * translation * rotation * scale;
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

/// get rotation
[[nodiscard]] const Math::Quaternion &Transform::GetRotation() const
{
    return mImpl->mRotation;
}

/// set rotation
void Transform::SetRotation(const Math::Quaternion &rotation)
{
    mImpl->mRotation = rotation;
}

/// get world matrix
void Transform::GetWorldMatrix(Math::Matrix44 &world) const
{
    return mImpl->GetWorldMatrix(world);
}
