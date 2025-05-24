//==============================================================================================================================================================================
/// \file
/// \brief     Application interface class
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifndef L_APPLICATION_H
#define L_APPLICATION_H

//#include "lMath.h"
#include "lFramework.h"

/// Lumen namespace
namespace Lumen
{

    /// Application interface class
    class Application
    {
    public:

        Application() noexcept(false) {}
        ~Application() {}

        Application(Application &&) = default;
        Application &operator= (Application &&) = default;

        Application(Application const &) = delete;
        Application &operator= (Application const &) = delete;

        };
}

#endif
