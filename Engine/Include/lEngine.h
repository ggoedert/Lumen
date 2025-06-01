//==============================================================================================================================================================================
/// \file
/// \brief     Engine interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_ENGINE_H
#define L_ENGINE_H

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    /// Engine class
    class Engine : public std::enable_shared_from_this<Engine>
    {
        CLASS_UTILS(Engine);
    };
}

#endif
