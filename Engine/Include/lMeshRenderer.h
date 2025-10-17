//==============================================================================================================================================================================
/// \file
/// \brief     MeshRenderer interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lComponent.h"
#include "lMaterial.h"
#include "lTexture.h"
#include "lSceneManager.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(MeshRenderer);
    CLASS_WEAK_PTR_DEF(MeshRenderer);
    CLASS_PTR_DEF(MeshFilter);

    /// MeshRenderer class
    class MeshRenderer : public Component
    {
        CLASS_NO_DEFAULT_CTOR(MeshRenderer);
        CLASS_NO_COPY_MOVE(MeshRenderer);
        COMPONENT_TYPEINFO;

    public:
        /// property value type
        using PropertyValue = std::variant<int, float, Lumen::TexturePtr>;

        /// mesh renderer creation parameters
        struct Params : Object
        {
            OBJECT_TYPEINFO;
            explicit Params(MaterialPtr material) : Object(Type()), mMaterial(material) {}
            MaterialPtr mMaterial;
        };

        /// set property
        void SetProperty(std::string_view name, const PropertyValue &property);

        /// get property
        [[nodiscard]] Expected<PropertyValue> GetProperty(std::string_view name) const;

        /// render
        void Render();

    private:
        /// constructs a mesh renderer with an material
        explicit MeshRenderer(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, MaterialPtr material);

        /// creates a smart pointer version of the mesh renderer component
        static ComponentPtr MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, const Object &params);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
