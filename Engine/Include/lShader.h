//==============================================================================================================================================================================
/// \file
/// \brief     Shader interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"
#include "lTexture.h"
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
        virtual ~Shader() noexcept override;

        /// release a shader
        void Release();

        /// set int property
        void SetProperty(std::string_view name, const int property);

        /// set float property
        void SetProperty(std::string_view name, const float property);

        /// set TexturePtr property
        void SetProperty(std::string_view name, const Lumen::TexturePtr &property);

    protected:
        /// constructs a shader
        explicit Shader(const EngineWeakPtr &engine);

        /// engine pointer
        EngineWeakPtr mEngine;

        /// engine shader id
        Engine::IdType mShaderId;

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
