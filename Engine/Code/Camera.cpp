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
    void Serialize(Serialized::Type &out, bool packed) const
    {
        // token
        std::string backgroundColorToken = packed ? Serialized::cBackgroundColorTokenPacked : Serialized::cBackgroundColorToken;

        // get background color
        if (mBackgroundColor != Math::Vector4::cZero)
        {
            out[backgroundColorToken] = { mBackgroundColor.x, mBackgroundColor.y, mBackgroundColor.z, mBackgroundColor.w };
        }

        // if empty, set to object
        if (out.empty())
        {
            out = Serialized::Type::object();
        }
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        // token
        std::string backgroundColorToken = packed ? Serialized::cBackgroundColorTokenPacked : Serialized::cBackgroundColorToken;

        // set background color
        mBackgroundColor = Math::Vector4::cZero;
        if (in.contains(backgroundColorToken))
        {
            auto arr = in[backgroundColorToken].get<std::vector<float>>();
            if (arr.size() != 4)
            {
                throw std::runtime_error(std::format("Unable to read Camera::BackgroundColor"));
            }
            mBackgroundColor = Math::Vector4(arr[0], arr[1], arr[2], arr[3]);
        }
    }

    /// get background color
    [[nodiscard]] const Math::Vector4 &GetBackgroundColor() const { return mBackgroundColor; }

    /// set background color
    void SetBackgroundColor(const Math::Vector4 &backgroundColor) { mBackgroundColor = backgroundColor; }

private:

    /// background color
    Math::Vector4 mBackgroundColor;
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
void Camera::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Camera::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// get background color
const Math::Vector4 &Camera::GetBackgroundColor() const
{
    return mImpl->GetBackgroundColor();
}

/// set background color
void Camera::SetBackgroundColor(const Math::Vector4 &backgroundColor)
{
    mImpl->SetBackgroundColor(backgroundColor);
}
