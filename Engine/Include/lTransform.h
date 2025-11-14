//==============================================================================================================================================================================
/// \file
/// \brief     Transform interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lMath.h"
#include "lSerializedData.h"
#include "lObject.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Transform);
    CLASS_WEAK_PTR_DEF(Transform);
    CLASS_WEAK_PTR_DEF(Entity);

    /// Transform class
    class Transform : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Transform);
        OBJECT_TYPEINFO;

    public:
        /// destructor
        ~Transform() override;

        /// creates a smart pointer version of the transform
        static TransformPtr MakePtr(const EntityWeakPtr &entity);

        /// serialize
        void Serialize(Serialized::Type &out, bool packed) const;

        /// deserialize
        void Deserialize(const Serialized::Type &in, bool packed);

        /// get owning entity
        [[nodiscard]] EntityWeakPtr Entity() const;

        /// get parent
        [[nodiscard]] const TransformWeakPtr &GetParent() const;

        /// set parent
        void SetParent(const TransformWeakPtr &parent);

        /// get position
        [[nodiscard]] const Math::Vector3 &GetPosition() const;

        /// set position
        void SetPosition(const Math::Vector3 &position);

        /// translate by x, y, z
        void Translate(float x, float y, float z);

        /// get rotation
        [[nodiscard]] const Math::Quaternion &GetRotation() const;

        /// set rotation
        void SetRotation(const Math::Quaternion &rotation);

        /// rotate by euler angles in degrees
        void Rotate(float xAngle, float yAngle, float zAngle);

        /// get world matrix
        void GetWorldMatrix(Math::Matrix44 &world) const;

    private:
        /// constructor
        explicit Transform(const EntityWeakPtr &entity);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
