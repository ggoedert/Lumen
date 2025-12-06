//==============================================================================================================================================================================
/// \file
/// \brief     Event implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEvent.h"

using namespace Lumen;

/// simple events
Lumen::HashType Event::AttachType = Lumen::EncodeType("Lumen::AttachEvent");
Lumen::HashType Event::DettachType = Lumen::EncodeType("Lumen::DettachEvent");

/// constructor
Event::Event(HashType type) : Object(type) {}

/// creates a smart pointer version of an event
EventPtr Event::MakePtr(HashType type)
{
    return EventPtr(new Event(type));
}
