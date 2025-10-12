//==============================================================================================================================================================================
/// \file
/// \brief     Material interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lComponent.h"
#include "lShader.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Material);

    /// Material class
    class Material : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Material);
        CLASS_NO_COPY_MOVE(Material);
        OBJECT_TYPEINFO;

    public:
        /// material creation parameters
        struct Params : Object
        {
            OBJECT_TYPEINFO;
            explicit Params(std::string_view shaderName) : Object(Type()), mShaderName(shaderName) {}
            std::string_view mShaderName;
        };

        /// creates a smart pointer version of the Material
        static Expected<MaterialPtr> MakePtr(std::string_view shaderName);

        /// get shader
        [[nodiscard]] ShaderPtr GetShader() const;

        /// set shader
        void SetShader(const ShaderPtr &shader);

    private:
        /// constructs a material with an shader
        explicit Material(ShaderPtr shader);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
