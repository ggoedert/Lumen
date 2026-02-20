//==============================================================================================================================================================================
/// \file
/// \brief     DrawPrimitive interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lId.h"
#include "lMath.h"
#include "lRenderCommand.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_UNIQUE_PTR_DEF(DrawPrimitive);

    /// DrawPrimitive class
    class DrawPrimitive : public RenderCommand
    {
        OBJECT_TYPEINFO;

    public:
        /// construct unique pointer
        static DrawPrimitiveUniquePtr MakeUniquePtr(Id::Type meshId, Id::Type shaderId, Id::Type texId, const Math::Matrix44 &world)
        {
            return DrawPrimitiveUniquePtr(new DrawPrimitive(meshId, shaderId, texId, world));
        };

        /// destroys draw primitive
        ~DrawPrimitive() override = default;

        /// mesh
        Id::Type mMeshId;

        /// shader
        Id::Type mShaderId;

        //texture
        Id::Type mTexId;

        //world matrix
        Math::Matrix44 mWorld;

    private:
        /// constructor
        explicit DrawPrimitive(Id::Type meshId, Id::Type shaderId, Id::Type texId, const Math::Matrix44 &world) :
            RenderCommand(Type()), mMeshId(meshId), mShaderId(shaderId), mTexId(texId), mWorld(world) {}
    };
}
