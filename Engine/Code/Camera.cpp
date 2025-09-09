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
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
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

DEFINE_COMPONENT_TYPEINFO(Camera);

/// constructs a camera with a background color
Camera::Camera(const GameObjectWeakPtr &gameObject, Math::Vector backgroundColor) :
    Behavior(Type(), Name(), gameObject), mImpl(Camera::Impl::MakeUniquePtr(backgroundColor)) {}

/// creates a smart pointer version of the camera component
ComponentPtr Camera::MakePtr(const GameObjectWeakPtr &gameObject, const Object &params)
{
    if (params.Type() == Camera::Params::Type())
    {
        const auto &createParams = static_cast<const Params &>(params);
        return ComponentPtr(new Camera(gameObject, createParams.mBackgroundColor));
    }
#ifdef TYPEINFO
    DebugLog::Error("Create component, unknown parameter type: {}", params.Type().mName);
#else
    DebugLog::Error("Create component, unknown parameter hash type: 0x{:08X}", params.Type());
#endif
    return {};
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
