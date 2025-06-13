//==============================================================================================================================================================================
/// \file
/// \brief     Behavior interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lComponent.h"

/// Lumen namespace
namespace Lumen
{
    /// Behavior class
    class Behavior : public Component
    {
        CLASS_NO_DEFAULT_CTOR(Behavior);
        CLASS_NO_COPY_MOVE(Behavior);

    public:
        /// constructor
        Behavior(Type componentType, const std::string &componentName, const GameObjectWeakPtr &gameObject) : Component(componentType, componentName, gameObject), mEnabled(true) {}

        /// control enabled
        void Enable(bool enable) { mEnabled = enable; }

        /// return enabled
        [[nodiscard]] bool Enabled() { return mEnabled; }

    private:
        /// behaviour enabled
        bool mEnabled;
    };
}
