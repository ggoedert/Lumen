//==============================================================================================================================================================================
/// \file
/// \brief     scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Scene);

    /// Scene class
    class Scene
    {
        CLASS_NO_COPY_MOVE(Scene);

    public:
        /// constructor
        explicit Scene() = default;

        /// virtual destructor
        virtual ~Scene() noexcept = default;

        /// load scene
        virtual bool Load() = 0;

        /// unload scene
        virtual void Unload() = 0;
    };
}
