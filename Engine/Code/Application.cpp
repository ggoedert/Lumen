//==============================================================================================================================================================================
/// \file
/// \brief     application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lApplication.h"
#include "lCamera.h"
#include "lSceneManager.h"

#ifdef EDITOR
#include "lEditor.h"
#endif

using namespace Lumen;

/// Application::Impl class
class Application::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Application;

public:
    /// constructs application
    explicit Impl();

    /// destructor
    ~Impl();

    /// get engine
    [[nodiscard]] EngineWeakPtr GetEngine() { return mEngine; }

    /// set engine
    void SetEngine(const EngineWeakPtr &engine);

    /// initialize application
    void Initialize(const Lumen::ApplicationWeakPtr &application);

    /// shutdown aplication
    void Shutdown();

    /// get delta time
    [[nodiscard]] float DeltaTime() const { return mDeltaTime; }

    /// get time
    [[nodiscard]] float Time() const { return mTime; }

    /// get background color
    [[nodiscard]] const Math::Vector4 &BackgroundColor() const;

protected:
#ifdef EDITOR
    /// run editor
    void Editor();
#endif

    /// run application
    bool Run(float deltaTime);

#ifdef EDITOR
    /// set application pause
    void Pause(bool pause);

    /// return if paused
    bool Paused();
#endif

    /// quit application
    void Quit();

private:
    /// default background color
    static constexpr Math::Vector4 cDefaultBackgroundColor { 1.f, 0.8f, 0.f, 1.f };

    /// application running
    bool mRunning { true };

#ifdef EDITOR
    /// application paused
    bool mPaused { false };
#endif

    /// engine pointer
    EngineWeakPtr mEngine;

#ifdef EDITOR
    /// editor pointer
    EditorPtr mEditor;
#endif

    /// interval in seconds from the last frame to the current one
    float mDeltaTime { 0.f };

    /// time at the beginning of the current frame
    float mTime { 0.f };
};

/// constructs application
Application::Impl::Impl() {}

/// destroys application
Application::Impl::~Impl() {}

/// set engine
void Application::Impl::SetEngine(const EngineWeakPtr &engine)
{
    mEngine = engine;
}

/// initialize application
void Application::Impl::Initialize(const Lumen::ApplicationWeakPtr &application)
{
#ifdef EDITOR
    mEditor = Editor::MakePtr(application);
    mEditor->Initialize();
#endif
}

/// shutdown aplication
void Application::Impl::Shutdown()
{
#ifdef EDITOR
    mEditor->Shutdown();
#endif
}

#ifdef EDITOR
/// run editor
void Application::Impl::Editor()
{
    mEditor->Run();
}
#endif

/// run application
bool Application::Impl::Run(float deltaTime)
{
    if (mRunning)
    {
#ifdef EDITOR
        if (mPaused)
        {
            deltaTime = 0.f;
        }
#endif
        mDeltaTime = deltaTime;
        mTime += mDeltaTime;
        SceneManager::Run();
        return true;
    }
    return false;
}

#ifdef EDITOR
/// set application pause
void Application::Impl::Pause(bool pause)
{
    mPaused = pause;
}

/// return if paused
bool Application::Impl::Paused()
{
    return mPaused;
}
#endif

/// quit application
void Application::Impl::Quit()
{
    mRunning = false;
}

/// get background color
const Math::Vector4 &Application::Impl::BackgroundColor() const
{
    Components cameras = SceneManager::GetComponents(Camera::Type());
    if (!cameras.empty())
    {
        if (ComponentPtr cameraPtr = cameras.front().lock())
        {
            return std::static_pointer_cast<Camera>(cameraPtr)->GetBackgroundColor();
        }
    }
    return cDefaultBackgroundColor;
}

//==============================================================================================================================================================================

/// constructs application
Application::Application() : mImpl(Application::Impl::MakeUniquePtr()) {}

/// virtual destructor
Application::~Application() {}

/// get engine
[[nodiscard]] EngineWeakPtr Application::GetEngine()
{
    return mImpl->GetEngine();
}

/// set engine
void Application::SetEngine(const EngineWeakPtr &engine)
{
    mImpl->SetEngine(engine);
}

/// initialize application
void Application::Initialize(const ApplicationWeakPtr &application)
{
    mImpl->Initialize(application);
}

/// shutdown aplication
void Application::Shutdown()
{
    mImpl->Shutdown();
}

/// get delta time
float Application::DeltaTime() const
{
    return mImpl->DeltaTime();
}

/// get time
float Application::Time() const
{
    return mImpl->Time();
}

/// get background color
const Math::Vector4 &Application::BackgroundColor() const
{
    return mImpl->BackgroundColor();
}

/// quit application
void Application::Quit()
{
    return mImpl->Quit();
}

#ifdef EDITOR
/// set application pause
void Application::Pause(bool pause)
{
    mImpl->Pause(pause);
}

/// return if paused
bool Application::Paused()
{
    return mImpl->Paused();
}

/// run editor
void Application::Editor()
{
    return mImpl->Editor();
}
#endif

/// run application
bool Application::Run(float deltaTime)
{
    return mImpl->Run(deltaTime);
}
