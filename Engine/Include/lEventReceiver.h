//==============================================================================================================================================================================
/// \file
/// \brief     EventReceiver interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lEvent.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(EventReceiver);
    CLASS_WEAK_PTR_DEF(EventReceiver);

    /// EventReceiver class
    class EventReceiver
    {
        CLASS_NO_COPY_MOVE(EventReceiver);

    public:
        /// handle event
        virtual bool HandleEvent(const EventPtr &event) = 0;

    protected:
        /// default constructor
        explicit EventReceiver() = default;

        /// pure virtual destructor
        virtual ~EventReceiver() = default;
    };
}
