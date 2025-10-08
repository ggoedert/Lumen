//==============================================================================================================================================================================
/// \file
/// \brief     Material interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lComponent.h"
#include "lShader.h"
#include "lSceneManager.h"
#include "lProperty.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Material);

    /// Material class
    class Material : public Component
    {
        CLASS_NO_DEFAULT_CTOR(Material);
        CLASS_NO_COPY_MOVE(Material);
        COMPONENT_TYPEINFO;
        friend void SceneManager::Initialize();

    public:
        /// property value type
        using PropertyValue = std::variant<int, float, Lumen::TexturePtr>;

        /// material creation parameters
        struct Params : Object
        {
            OBJECT_TYPEINFO;
            explicit Params(std::string_view shaderName) : Object(Type()), mShaderName(shaderName) {}
            std::string_view mShaderName;
        };

        /// set property
        void SetProperty(std::string_view name, const PropertyValue &property);

        /// get property
        [[nodiscard]] Expected<PropertyValue> GetProperty(std::string_view name) const;

    private:
        /// constructs a material with an shader
        explicit Material(const GameObjectWeakPtr &gameObject, ShaderPtr shader);

        /// creates a smart pointer version of the material component
        static ComponentPtr MakePtr(const GameObjectWeakPtr &gameObject, const Object &params);

        /// run component
        void Run() override;

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
