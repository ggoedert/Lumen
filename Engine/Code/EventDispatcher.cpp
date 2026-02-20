//==============================================================================================================================================================================
/// \file
/// \brief     EventDispatcher, tranverses nodes dispatching, either top to bottom or bottom to top
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lEventDispatcher.h"
#include "lEvent.h"

using namespace Lumen;

/// EventDispatcher::Impl class
class EventDispatcher::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class EventDispatcher;

public:
    /// constructs an event dispatcher
    explicit Impl(EventDispatcher &owner) : mOwner(owner) {}

    /// attach an event receiver
    void Attach(const EventReceiverPtr &receiver)
    {
        mReceivers.push_back(receiver);
        receiver->HandleEvent(Event::MakePtr(AttachType));
    }

    /// detach an event receiver
    void Detach(const EventReceiverPtr &receiver)
    {
        Lumen::RemoveFromVector(mReceivers, receiver);
        receiver->HandleEvent(Event::MakePtr(DettachType));
    }

    /// broadcast to all event receivers, ignore if handled
    void Broadcast(const EventPtr &event) const
    {
        for (auto &receiver : mReceivers)
        {
            receiver->HandleEvent(event);
        }
    }

    /// traverse all event receivers from top to bottom, stop if handled
    bool Traverse(const EventPtr &event) const
    {
        for (auto &receiver : mReceivers)
        {
            if (receiver->HandleEvent(event))
            {
                return true;
            }
        }
        return false;
    }

    /// traverse all event receivers from bottom to top, stop if handled
    bool ReverseTraverse(const EventPtr &event) const
    {
        for (auto it = mReceivers.rbegin(); it != mReceivers.rend(); ++it)
        {
            if ((*it)->HandleEvent(event))
            {
                return true;
            }
        }
        return false;
    }

private:
    /// owner
    EventDispatcher &mOwner;

    /// receivers
    std::vector<EventReceiverPtr> mReceivers;
};

//==============================================================================================================================================================================

/// simple events
Lumen::HashType EventDispatcher::AttachType = Lumen::EncodeType("EventDispatcher::AttachEvent");
Lumen::HashType EventDispatcher::DettachType = Lumen::EncodeType("EventDispatcher::DettachEvent");

/// constructs an event dispatcher
EventDispatcher::EventDispatcher() : mImpl(EventDispatcher::Impl::MakeUniquePtr(*this)) {}

/// creates a smart pointer version of an event dispatcher
EventDispatcherPtr EventDispatcher::MakePtr()
{
    return EventDispatcherPtr(new EventDispatcher());
}

/// attach an event receiver
void EventDispatcher::Attach(const EventReceiverPtr &receiver)
{
    mImpl->Attach(receiver);
}

/// detach an event receiver
void EventDispatcher::Detach(const EventReceiverPtr &receiver)
{
    mImpl->Detach(receiver);
}

/// broadcast to all event receivers, ignore if handled
void EventDispatcher::Broadcast(const EventPtr &event) const
{
    mImpl->Broadcast(event);
}

/// traverse all event receivers from top to bottom, stop if handled
bool EventDispatcher::Traverse(const EventPtr &event) const
{
    return mImpl->Traverse(event);
}

/// traverse all event receivers from bottom to top, stop if handled
bool EventDispatcher::ReverseTraverse(const EventPtr &event) const
{
    return mImpl->ReverseTraverse(event);
}
