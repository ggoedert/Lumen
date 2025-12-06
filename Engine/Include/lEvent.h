//==============================================================================================================================================================================
/// \file
/// \brief     Event interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Event);

    /// Event class
    class Event : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Event);
        CLASS_NO_COPY_MOVE(Event);

    public:
        /// simple events
        static Lumen::HashType AttachType;
        static Lumen::HashType DettachType;

        /// creates a smart pointer version of the event
        static EventPtr MakePtr(HashType type);

    protected:
        /// constructor
        explicit Event(HashType type);
    };

}
