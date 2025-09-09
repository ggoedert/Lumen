//==============================================================================================================================================================================
/// \file
/// \brief     Shader interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <variant>

#include "lDefs.h"
#include "lObject.h"
#include "lTexture.h"

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
        /// destroys shader
        ~Shader() noexcept override;

        /// set int property
        void SetProperty(std::string_view name, const int property);

        /// set float property
        void SetProperty(std::string_view name, const float property);

        /// set TexturePtr property
        void SetProperty(std::string_view name, const Lumen::TexturePtr &property);

    protected:
        /// constructs a shader
        explicit Shader();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
