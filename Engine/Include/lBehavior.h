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
    CLASS_WEAK_PTR_DEF(Behavior);

    /// Behavior class
    class Behavior : public Component
    {
        CLASS_NO_DEFAULT_CTOR(Behavior);
        CLASS_NO_COPY_MOVE(Behavior);

    public:
        /// constructor
        explicit Behavior(const HashType type, std::string_view name, const GameObjectWeakPtr &gameObject);

        /// destructor
        ~Behavior() noexcept override;

        /// control enabled
        void Enable(bool enable);

        /// return enabled
        [[nodiscard]] bool Enabled();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
