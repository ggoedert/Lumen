//==============================================================================================================================================================================
/// \file
/// \brief     application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lApplication.h"
#include "lCamera.h"
#include "lBuiltinResources.h"

using namespace Lumen;

/// Application::Impl class
class Application::Impl
{
    CLASS_NO_COPY_MOVE(Application::Impl);
    CLASS_PTR_UNIQUEMAKER(Application::Impl);
    friend class Application;

public:
    /// constructs application
    explicit Impl();

    /// destructor
    ~Impl() noexcept;

    /// get engine
    [[nodiscard]] EngineWeakPtr GetEngine() { return mEngine; }

    /// set engine
    void SetEngine(const EngineWeakPtr &engine) { mEngine = engine; }

    /// initialize application
    [[nodiscard]] bool Initialize();

    /// shutdown application
    void Shutdown();

    /// get assets
    [[nodiscard]] Lumen::Assets &Assets() { return mAssets; }

    /// get delta time
    [[nodiscard]] float DeltaTime() const { return mDeltaTime; }

    /// get time
    [[nodiscard]] float Time() const { return mTime; }

    /// get background color
    [[nodiscard]] const Math::Vector GetBackgroundColor() const;

protected:
    /// run application
    bool Run(float deltaTime);

    /// quit application
    void Quit();

private:
    /// default background color
    static constexpr Math::Vector cDefaultBackgroundColor { 1.f, 0.8f, 0.f, 1.f };

    /// application running
    bool mRunning { true };

    /// assets
    Lumen::Assets mAssets;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// interval in seconds from the last frame to the current one
    float mDeltaTime { 0.f };

    /// time at the beginning of the current frame
    float mTime { 0.f };
};

/// constructs application
Application::Impl::Impl() {}

/// destroys application
Application::Impl::~Impl() noexcept {}

/// initialize application
bool Application::Impl::Initialize()
{
    Assets().RegisterFactory(DefaultResources::MakePtr(), "", 1.0f);
    return true;
}

/// shutdown application
void Application::Impl::Shutdown()
{
}

/// run application
bool Application::Impl::Run(float deltaTime)
{
    if (mRunning)
    {
        mDeltaTime = deltaTime;
        mTime += mDeltaTime;
        SceneManager::Run();
        return true;
    }
    return false;
}

/// quit application
void Application::Impl::Quit()
{
    mRunning = false;
}

/// get background color
const Math::Vector Application::Impl::GetBackgroundColor() const
{
    Components cameras = SceneManager::GetComponents(Camera::Type());
    if (!cameras.empty())
    {
        if (ComponentPtr cameraPtr = cameras.front().lock())
        {
            return std::static_pointer_cast<Camera>(cameraPtr)->BackgroundColor();
        }
    }
    return cDefaultBackgroundColor;
}

//==============================================================================================================================================================================

/// constructs application
Application::Application() : mImpl(Application::Impl::MakeUniquePtr()) {}

/// virtual destructor
Application::~Application() noexcept {}

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
bool Application::Initialize()
{
    return mImpl->Initialize();
}

/// shutdown application
void Application::Shutdown()
{
    return mImpl->Shutdown();
}

/// get assets
Assets &Application::Assets() const
{
    return mImpl->Assets();
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
const Math::Vector Application::BackgroundColor() const
{
    return mImpl->GetBackgroundColor();
}

/// run application
bool Application::Run(float deltaTime)
{
    return mImpl->Run(deltaTime);
}

/// quit application
void Application::Quit()
{
    return mImpl->Quit();
}
