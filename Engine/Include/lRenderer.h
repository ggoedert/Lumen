//==============================================================================================================================================================================
/// \file
/// \brief     Renderer interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lComponent.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Renderer);
    CLASS_WEAK_PTR_DEF(Renderer);

    /// Renderer class
    class Renderer : public Component
    {
        CLASS_NO_DEFAULT_CTOR(Renderer);
        CLASS_NO_COPY_MOVE(Renderer);
        COMPONENT_TYPEINFO;

    public:
        /// serialize
        void Serialize(Serialized::Type &out, bool packed) const override;

        /// deserialize
        void Deserialize(const Serialized::Type &in, bool packed) override;

        /// render
        void Render();

    private:
        /// constructs a renderer with an material
        explicit Renderer(const EngineWeakPtr &engine, const EntityWeakPtr &entity);

        /// creates a smart pointer version of the renderer component
        static ComponentPtr MakePtr(const EngineWeakPtr &engine, const EntityWeakPtr &entity);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
