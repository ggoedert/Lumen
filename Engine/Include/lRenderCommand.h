//==============================================================================================================================================================================
/// \file
/// \brief     Render Command interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lObject.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_UNIQUE_PTR_DEF(RenderCommand);

    /// Render command class
    class RenderCommand : public Object
    {
        CLASS_NO_DEFAULT_CTOR(RenderCommand);
        CLASS_NO_COPY_MOVE(RenderCommand);

    public:
        /// destroys render command
        ~RenderCommand() override = default;

    protected:
        /// constructor
        explicit RenderCommand(HashType type) : Object(type) {}
    };
}
