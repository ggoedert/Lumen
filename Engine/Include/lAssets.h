//==============================================================================================================================================================================
/// \file
/// \brief     assets interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lObject.h"

/// Lumen namespace
namespace Lumen
{
    /// Assets class
    class Assets
    {
        CLASS_NO_COPY_MOVE(Assets);

    public:
        /// constructor
        explicit Assets();

        /// destructor
        ~Assets() noexcept;

        /// import asset
        ObjectPtr Import(std::string_view path, const HashType type, std::string_view name);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
