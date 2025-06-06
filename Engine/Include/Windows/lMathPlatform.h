//==============================================================================================================================================================================
/// \file
/// \brief     Math windows platform
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "SimpleMath.h"

/// Lumen namespace
namespace Lumen
{
    /// Math namespace (Windows platform specific)
    namespace Math
    {
        /// alias to DirectX 12 helper classes
        using Vector = DirectX::XMVECTORF32;
        using Vector2 = DirectX::SimpleMath::Vector2;
        using Vector3 = DirectX::SimpleMath::Vector3;
        using Vector4 = DirectX::SimpleMath::Vector4;
    }
}
