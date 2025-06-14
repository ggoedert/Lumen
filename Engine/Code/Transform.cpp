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
    CLASS_NO_DEFAULT_CTOR(Transform::Impl);
    CLASS_NO_COPY_MOVE(Transform::Impl);
    CLASS_PTR_UNIQUEMAKER(Transform::Impl);

public:
    /// constructs a transform
    Impl(const GameObjectWeakPtr &gameObject) : mGameObject(gameObject) {}

    /// destroys transform
    ~Impl() = default;

    /// get owning game object
    [[nodiscard]] GameObjectWeakPtr GameObject() const { return mGameObject; }

    /// get parent
    [[nodiscard]] TransformWeakPtr GetParent() const { return mParent; }

    /// set parent
    void SetParent(TransformWeakPtr parent)
    {
        LUMEN_ASSERT(parent.lock() != mInterface.lock());
        mParent = parent;
    }

    /// get position
    [[nodiscard]] Math::Vector3 GetPosition() const { return mPosition; }

    /// set position
    void SetPosition(const Math::Vector3 &position) { mPosition = position; }

private:
    /// interface
    TransformWeakPtr mInterface;

    /// owning game object
    GameObjectWeakPtr mGameObject;

    /// parent transform
    TransformWeakPtr mParent;

    /// position
    Math::Vector3 mPosition;
};

//==============================================================================================================================================================================

/// constructor
Transform::Transform(const GameObjectWeakPtr &gameObject) : Object(), mImpl(Transform::Impl::MakeUniquePtr(gameObject)) {}

/// destructor
Transform::~Transform() = default;

/// get owning game object
[[nodiscard]] GameObjectWeakPtr Transform::GameObject() const
{
    return mImpl->GameObject();
}

/// get parent
TransformWeakPtr Transform::GetParent() const
{
    return mImpl->GetParent();
}

/// set parent
void Transform::SetParent(TransformWeakPtr parent)
{
    mImpl->SetParent(parent);
}

/// get position
[[nodiscard]] Math::Vector3 Transform::GetPosition() const
{
    return mImpl->GetPosition();
}

/// set position
void Transform::SetPosition(const Math::Vector3 &position)
{
    mImpl->SetPosition(position);
}
