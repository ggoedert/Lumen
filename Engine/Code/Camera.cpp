//==============================================================================================================================================================================
/// \file
/// \brief     Camera
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lCamera.h"

using namespace Lumen;

/// Camera::Impl class
class Camera::Impl
{
    CLASS_NO_DEFAULT_CTOR(Camera::Impl);
    CLASS_NO_COPY_MOVE(Camera::Impl);
    CLASS_PTR_UNIQUEMAKER(Camera::Impl);
    friend class Camera;

public:
    /// constructs a camera
    Impl(Math::Vector backgroundColor) : mBackgroundColor(backgroundColor) {}

    /// set background color
    void SetBackgroundColor(Math::Vector &backgroundColor) { mBackgroundColor = backgroundColor; }

    /// get background color
    [[nodiscard]] Math::Vector GetBackgroundColor() const { return mBackgroundColor; }

private:
    /// run component
    void Run() {}

    /// background color
    Math::Vector mBackgroundColor;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TRAITS(Camera);

/// constructs a camera with name and background color
Camera::Camera(const GameObjectWeakPtr &gameObject, Math::Vector backgroundColor) :
    Behavior(ComponentType(), ComponentName(), gameObject), mImpl(Camera::Impl::MakeUniquePtr(backgroundColor)) {}

/// set background color
void Camera::SetBackgroundColor(Math::Vector &backgroundColor) { mImpl->SetBackgroundColor(backgroundColor); }

/// get background color
[[nodiscard]] Math::Vector Camera::GetBackgroundColor() const { return mImpl->GetBackgroundColor(); }

/// run component
void Camera::Run() { mImpl->Run(); }
