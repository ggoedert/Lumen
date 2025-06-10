//==============================================================================================================================================================================
/// \file
/// \brief     Component
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lComponent.h"
#include "lGameObject.h"

using namespace Lumen;

/// constructs a component with type, name, and parent. called by derived classes
Component::Component(Type componentType, const std::string &componentName, const GameObjectWeakPtr &gameObject) :
    Object(), mComponentType(componentType), mComponentName(componentName), mGameObject(gameObject)
{
}
