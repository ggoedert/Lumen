//==============================================================================================================================================================================
/// \file
/// \brief     application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lApplication.h"
#include "lCamera.h"

using namespace Lumen;

/// constructs application
Application::Application() : mSceneManager(new SceneManager(*this)) {}

/// destroys application
Application::~Application() noexcept
{
    delete mSceneManager;
}

/// run application
bool Application::Run(float deltaTime)
{
    if (mRunning)
    {
        mDeltaTime = deltaTime;
        mTime += mDeltaTime;
        mSceneManager->Run();
        return true;
    }
    return false;
}

/// quit application
void Application::Quit()
{
    mRunning = false;
}

/// get background color
const Math::Vector &Application::GetBackgroundColor()
{
    std::vector<Component *> &cameras = GetSceneManager().GetComponents(Camera::ComponentType());
    if (!cameras.empty())
    {
        return static_cast<Camera *>(cameras.front())->BackgroundColor();
    }
    return cDefaultBackgroundColor;
}
