//==============================================================================================================================================================================
/// \file
/// \brief     Application interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lSceneManager.h"
#include "lEngine.h"

/// Lumen namespace
namespace Lumen
{
    class SceneManager;
    CLASS_PTR_DEFS(Application);

    /// Application class
    class Application
    {
        CLASS_NO_COPY_MOVE(Application);
        CLASS_PTR_MAKERS(Application);

    public:
        /// set engine
        [[nodiscard]] void SetEngine(EnginePtr engine) { mEngine = engine; }

        /// get scene manager
        [[nodiscard]] SceneManager &GetSceneManager() { return *mSceneManager; }

        /// load application
        virtual bool Load() = 0;

        /// unload application
        virtual void Unload() = 0;

        /// get delta time
        [[nodiscard]] float &GetDeltaTime() { return mDeltaTime; }

        /// get time
        [[nodiscard]] float &GetTime() { return mTime; }

        /// get background color
        const Math::Vector &GetBackgroundColor();

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
        /// Scene manager
        SceneManager *mSceneManager;

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
