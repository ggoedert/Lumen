//==============================================================================================================================================================================
/// \file
/// \brief     Application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lMath.h"
#include "lSceneManager.h"
#include "lEngine.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Engine);
    CLASS_WEAK_PTR_DEF(Engine);

    /// Application class
    class Application
    {
        CLASS_NO_COPY_MOVE(Application);

    public:
        /// set engine
        [[nodiscard]] void SetEngine(const EnginePtr &engine) { mEngine = engine; }

        /// load application
        virtual bool Load() = 0;

        /// unload application
        virtual void Unload() = 0;

        /// get delta time
        [[nodiscard]] float GetDeltaTime() const { return mDeltaTime; }

        /// get time
        [[nodiscard]] float GetTime() const { return mTime; }

        /// get background color
        const Math::Vector &GetBackgroundColor() const;

    protected:
        /// constructs application
        Application();

        /// virtual destructor
        virtual ~Application() noexcept;

        /// run application
        bool Run(float deltaTime);

        /// quit application
        void Quit();

    private:
        /// default background color
        static constexpr Math::Vector cDefaultBackgroundColor { 1.f, 0.8f, 0.f, 1.f };

        /// application running
        bool mRunning { true };

        /// engine pointer
        EngineWeakPtr mEngine;

        /// interval in seconds from the last frame to the current one
        float mDeltaTime { 0.f };

        /// time at the beginning of the current frame
        float mTime { 0.f };
    };
}
