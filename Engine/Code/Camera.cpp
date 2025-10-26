//==============================================================================================================================================================================
/// \file
/// \brief     Camera
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lCamera.h"
#include "lSceneManager.h"

using namespace Lumen;

/// Camera::Impl class
class Camera::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Camera;

public:
    /// constructs a camera
    explicit Impl() = default;

    /// serialize
    void Serialize(json &out) const
    {
    }

    /// deserialize
    void Deserialize(const json &in)
    {
        // set background color
        mBackgroundColor = {};
        if (in.contains("BackgroundColor"))
        {
            auto arr = in["BackgroundColor"].get<std::vector<float>>();
            if (arr.size() != 4)
            {
                throw std::runtime_error(std::format("Unable to read Camera::BackgroundColor"));
            }
            mBackgroundColor = Math::Vector4(arr[0], arr[1], arr[2], arr[3]);
        }
    }

    /// get background color
    [[nodiscard]] const Math::Vector &GetBackgroundColor() const { return mBackgroundColor; }

    /// set background color
    void SetBackgroundColor(const Math::Vector &backgroundColor) { mBackgroundColor = backgroundColor; }

private:
    /// background color
    Math::Vector mBackgroundColor;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(Camera);

/// constructs a camera with a background color
Camera::Camera(const GameObjectWeakPtr &gameObject) :
    Component(Type(), Name(), gameObject), mImpl(Camera::Impl::MakeUniquePtr()) {}

/// creates a smart pointer version of the camera component
ComponentPtr Camera::MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject)
{
    return ComponentPtr(new Camera(gameObject));
}

/// serialize
void Camera::Serialize(json &out) const
{
    mImpl->Serialize(out);
}

/// deserialize
void Camera::Deserialize(const json &in)
{
    mImpl->Deserialize(in);
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
