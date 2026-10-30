//==============================================================================================================================================================================
/// \file
/// \brief     Asset interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lSerializedData.h"
#include "lObject.h"

/// \cond
#include <filesystem>
/// \endcond

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
        virtual bool Save() const = 0;

        /// load asset
        virtual bool Load() = 0;

        /// release a asset
        virtual void Release() = 0;

        /// whether the serialized form of this asset can be cached in the Library
        [[nodiscard]] virtual bool SupportsLibrary() const { return false; }

        /// whether loads of the same path can share one instance
        [[nodiscard]] virtual bool Shareable() const { return false; }

        /// serialize
        virtual void Serialize(Serialized::Type &out, bool packed) const {}

        /// deserialize
        virtual void Deserialize(const Serialized::Type &in, bool packed) {}

        /// get path
        const std::filesystem::path &Path() const;

        /// get the UUID
        [[nodiscard]] UUID GetUUID() const;

        /// set the UUID
        void SetUUID(UUID uuid);

    protected:
        /// constructs a asset
        explicit Asset(HashType type, const std::filesystem::path &path);

        /// destroys asset
        ~Asset() override;

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
