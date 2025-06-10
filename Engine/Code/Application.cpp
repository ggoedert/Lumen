//==============================================================================================================================================================================
/// \file
/// \brief     application
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lApplication.h"
#include "lCamera.h"

using namespace Lumen;

/// constructs application
Application::Application() {}

/// destroys application
Application::~Application() noexcept {}

/// run application
bool Application::Run(float deltaTime)
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
void Application::Quit()
{
    mRunning = false;
}

/// get background color
const Math::Vector &Application::GetBackgroundColor() const
{
    Components cameras = SceneManager::GetComponents(Camera::ComponentType());
    if (!cameras.empty())
    {
        if (ComponentPtr cameraPtr = cameras.front().lock())
        {
            return static_cast<Camera *>(cameraPtr.get())->BackgroundColor();
        }
    }
    return cDefaultBackgroundColor;
}
