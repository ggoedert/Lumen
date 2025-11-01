//==============================================================================================================================================================================
/// \file
/// \brief     Asset interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lObject.h"

#include <filesystem>

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Asset);
    CLASS_WEAK_PTR_DEF(Asset);

    /// Asset class
    class Asset : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Asset);
        CLASS_NO_COPY_MOVE(Asset);

    public:
        /// destroys asset
        ~Asset() override;

        /// release a asset
        void Release();

        /// get path
        const std::filesystem::path &Path();

        /// get name
        const std::string &Name();

    protected:
        /// constructs a asset
        explicit Asset(HashType type, const std::filesystem::path &path, std::string_view name);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
