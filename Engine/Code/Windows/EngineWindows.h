//==============================================================================================================================================================================
/// \file
/// \brief     Engine windows implementation
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "EnginePlatform.h"

/// Lumen Windows namespace
namespace Lumen::Windows
{
    /// Engine windows class
    class EngineWindows : public EnginePlatform
    {
        CLASS_NO_COPY_MOVE(EngineWindows);

    public:
        /// simple windows events
        static Lumen::HashType DisplayChanged;
        static Lumen::HashType WindowMoved;
        static Lumen::HashType Suspending;
        static Lumen::HashType Resuming;
        static Lumen::HashType Activated;
        static Lumen::HashType Deactivated;

        /// window size changed event
        CLASS_UNIQUE_PTR_DEF(WindowSizeChanged);
        class WindowSizeChanged : public Event
        {
            OBJECT_TYPEINFO;
        public:
            static EventUniquePtr MakeUniquePtr(int width, int height) { return EventUniquePtr(new WindowSizeChanged(width, height)); }
            int mWidth;
            int mHeight;
        private:
            explicit WindowSizeChanged(int width, int height) : Event(Type()), mWidth(width), mHeight(height) {};
        };

    protected:
        /// constructs an engine
        explicit EngineWindows();

        /// destroys engine
        ~EngineWindows() override;

        /// initialization and management
        bool Initialize(const Object &config) override;

        /// pop all batches of items
        bool PopAssetChangeBatchQueue(std::list<std::vector<AssetManager::AssetChange>> &batchQueue) override;

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
