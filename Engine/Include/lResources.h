//==============================================================================================================================================================================
/// \file
/// \brief     resources interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lObject.h"

/// Lumen namespace
namespace Lumen
{
    /// Resources class
    class Resources
    {
        CLASS_NO_COPY_MOVE(Resources);

    public:
        /// constructor
        explicit Resources();

        /// virtual destructor
        ~Resources() noexcept;

        /// import resource
        ObjectPtr Import(std::string_view path, const HashType type, std::optional<std::string_view> name = std::nullopt);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
