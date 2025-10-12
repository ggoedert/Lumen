//==============================================================================================================================================================================
/// \file
/// \brief     Shader interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"
#include "lEngine.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Shader);

    /// Shader class
    class Shader : public Object
    {
        CLASS_NO_COPY_MOVE(Shader);
        OBJECT_TYPEINFO;

    public:
        /// custom smart pointer maker
        static Expected<ShaderPtr> MakePtr(std::string_view shaderName);

        /// destroys shader
        ~Shader() override;

        /// release a shader
        void Release();

        /// get shader id
        Id::Type GetShaderId();

        /// set shader id
        void SetShaderId(Id::Type shaderId);

    protected:
        /// constructs a shader
        explicit Shader(const EngineWeakPtr &engine);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
