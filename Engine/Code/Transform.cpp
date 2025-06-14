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
    [[nodiscard]] GameObjectWeakPtr GetGameObject() const { return mGameObject; }

    /// set parent
    void SetParent(TransformWeakPtr parent)
    {
        LUMEN_ASSERT(parent.lock() != mInterface.lock());
        mParent = parent;
    }

    /// get parent
    [[nodiscard]] TransformWeakPtr GetParent() const { return mParent; }

    /// set position
    void SetPosition(const Math::Vector3 &position) { mPosition = position; }

    /// get position
    [[nodiscard]] Math::Vector3 GetPosition() const { return mPosition; }

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
[[nodiscard]] GameObjectWeakPtr Transform::GetGameObject() const
{
    return mImpl->GetGameObject();
}

/// set parent
void Transform::SetParent(TransformWeakPtr parent)
{
    mImpl->SetParent(parent);
}

/// get parent
TransformWeakPtr Transform::GetParent() const
{
    return mImpl->GetParent();
}

/// set position
void Transform::SetPosition(const Math::Vector3 &position)
{
    mImpl->SetPosition(position);
}

/// get position
[[nodiscard]] Math::Vector3 Transform::GetPosition() const
{
    return mImpl->GetPosition();
}
