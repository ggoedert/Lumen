//==============================================================================================================================================================================
/// \file
/// \brief     Shader interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"

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

        /// set property
        void SetProperty(const std::string name, const Lumen::ObjectPtr &property);

    protected:
        /// constructs a shader
        explicit Shader();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
