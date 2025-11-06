//==============================================================================================================================================================================
/// \file
/// \brief     Asset interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lSerializedData.h"
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
        /// save asset
        virtual bool Save() const { return true; }//; = 0;

        /// load asset
        virtual bool Load() { return true; }//; = 0;

        /// release a asset
        virtual void Release() {}//; = 0;

        /// get name
        const std::string &Name();

        /// get path
        const std::filesystem::path &Path();

    protected:
        /// constructs a asset
        explicit Asset(HashType type, std::string_view name, const std::filesystem::path &path);

        /// destroys asset
        ~Asset() override;

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
