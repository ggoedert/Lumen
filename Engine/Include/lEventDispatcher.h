//==============================================================================================================================================================================
/// \file
/// \brief     EventDispatcher interface, tranverses nodes dispatching, either top to bottom or bottom to top
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lEventReceiver.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(EventDispatcher);
    CLASS_WEAK_PTR_DEF(EventDispatcher);

    /// EventDispatcher class
    class EventDispatcher : public std::enable_shared_from_this<EventDispatcher>
    {
        CLASS_NO_COPY_MOVE(EventDispatcher);

    public:
        /// simple events
        static Lumen::HashType AttachType;
        static Lumen::HashType DettachType;

        /// creates a smart pointer version of the event dispatcher
        static EventDispatcherPtr MakePtr();

        /// attach an event receiver
        void Attach(const EventReceiverPtr &receiver);

        /// detach an event receiver
        void Detach(const EventReceiverPtr &receiver);

        /// broadcast to all event receivers, ignore if handled
        void Broadcast(const EventPtr &event) const;

        /// traverse all event receivers from top to bottom, stop if handled
        bool Traverse(const EventPtr &event) const;

        /// traverse all event receivers from bottom to top, stop if handled
        bool ReverseTraverse(const EventPtr &event) const;

    private:
        /// constructor
        explicit EventDispatcher();

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
