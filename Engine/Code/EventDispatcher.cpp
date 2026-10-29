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
    CLASS_NO_DEFAULT_CTOR(Impl);
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
        if (mDispatchDepth > 0)
        {
            // a dispatch is walking the receivers by index, erasing would shift them under it:
            // clear the slot now (dispatching skips empty slots) and compact when the outermost dispatch ends
            for (auto &slot : mReceivers)
            {
                if (slot == receiver)
                {
                    slot.reset();
                    mCompact = true;
                }
            }
        }
        else
        {
            Lumen::RemoveFromVector(mReceivers, receiver);
        }
        receiver->HandleEvent(Event::MakePtr(DettachType));
    }

    /// broadcast to all event receivers, ignore if handled
    void Broadcast(const EventPtr &event) const
    {
        DispatchScope scope(*this);

        // receivers attached while dispatching do not see this event
        const size_t count = mReceivers.size();
        for (size_t i = 0; i < count; ++i)
        {
            // one reference count bump per receiver (no copy of the receiver list), keeps a receiver that detaches itself alive while it handles the event
            const EventReceiverPtr receiver = mReceivers[i];
            if (receiver)
            {
                receiver->HandleEvent(event);
            }
        }
    }

    /// traverse all event receivers from top to bottom, stop if handled
    bool Traverse(const EventPtr &event) const
    {
        DispatchScope scope(*this);

        const size_t count = mReceivers.size();
        for (size_t i = 0; i < count; ++i)
        {
            const EventReceiverPtr receiver = mReceivers[i];
            if (receiver && receiver->HandleEvent(event))
            {
                return true;
            }
        }
        return false;
    }

    /// traverse all event receivers from bottom to top, stop if handled
    bool ReverseTraverse(const EventPtr &event) const
    {
        DispatchScope scope(*this);

        for (size_t i = mReceivers.size(); i-- > 0;)
        {
            const EventReceiverPtr receiver = mReceivers[i];
            if (receiver && receiver->HandleEvent(event))
            {
                return true;
            }
        }
        return false;
    }

private:
    /// marks a dispatch in progress, detaching during a dispatch is deferred until the outermost dispatch ends
    class DispatchScope
    {
        CLASS_NO_COPY_MOVE(DispatchScope);

    public:
        /// enter a dispatch
        explicit DispatchScope(const Impl &impl) : mImpl(impl) { ++mImpl.mDispatchDepth; }

        /// leave a dispatch
        ~DispatchScope()
        {
            if ((--mImpl.mDispatchDepth == 0) && mImpl.mCompact)
            {
                mImpl.mCompact = false;
                std::erase_if(mImpl.mReceivers, [](const EventReceiverPtr &receiver) { return !receiver; });
            }
        }

    private:
        /// dispatcher implementation
        const Impl &mImpl;
    };

    /// owner
    EventDispatcher &mOwner;

    /// receivers, a slot is empty when its receiver detached during a dispatch (mutable: dispatching is logically const, it only compacts the list)
    mutable std::vector<EventReceiverPtr> mReceivers;

    /// number of dispatches in progress
    mutable int mDispatchDepth = 0;

    /// whether empty slots need to be removed once the dispatches are done
    mutable bool mCompact = false;
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
