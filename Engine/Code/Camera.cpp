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
    explicit Impl(Math::Vector backgroundColor) : mBackgroundColor(backgroundColor) {}

    /// get background color
    [[nodiscard]] const Math::Vector &GetBackgroundColor() const { return mBackgroundColor; }

    /// set background color
    void SetBackgroundColor(const Math::Vector &backgroundColor) { mBackgroundColor = backgroundColor; }

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
    Behavior(Type(), Name(), gameObject), mImpl(Camera::Impl::MakeUniquePtr(backgroundColor)) {}

/// creates a smart pointer version of the camera component
ComponentPtr Camera::MakePtr(const GameObjectWeakPtr &gameObject, const std::any &params)
{
    if (params.type() == typeid(Params))
    {
        const auto &createParams = std::any_cast<const Params &>(params);
        return ComponentPtr(new Camera(gameObject, createParams.backgroundColor));
    }
    DebugLog::Error("Create component, unknown parameter type: " + std::string(params.type().name()));
    return ComponentPtr();
}

/// get background color
const Math::Vector &Camera::GetBackgroundColor() const
{
    return mImpl->GetBackgroundColor();
}

/// set background color
void Camera::SetBackgroundColor(const Math::Vector &backgroundColor)
{
    mImpl->SetBackgroundColor(backgroundColor);
}

/// run component
void Camera::Run() { mImpl->Run(); }
