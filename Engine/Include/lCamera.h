//==============================================================================================================================================================================
/// \file
/// \brief     Camera interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lComponent.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Camera);
    CLASS_WEAK_PTR_DEF(Camera);

    /// Camera class
    class Camera : public Component
    {
        CLASS_NO_DEFAULT_CTOR(Camera);
        CLASS_NO_COPY_MOVE(Camera);
        COMPONENT_TYPEINFO;

    public:
        /// serialize
        void Serialize(Serialized::Type &out, bool packed) const override;

        /// deserialize
        void Deserialize(const Serialized::Type &in, bool packed) override;

        /// get the camera's background color
        [[nodiscard]] const Math::Vector4 &GetBackgroundColor() const;

        /// set the camera's background color
        void SetBackgroundColor(const Math::Vector4 &backgroundColor);

    private:
        /// constructs a camera with a background color
        explicit Camera(const GameObjectWeakPtr &gameObject);

        /// creates a smart pointer version of the camera component
        static ComponentPtr MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
