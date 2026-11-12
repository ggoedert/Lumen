//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lUUID.h"
#include "lHash.h"
#include "lSerializedData.h"

/// \cond
#include <filesystem>
/// \endcond

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(AssetInfo);
    CLASS_WEAK_PTR_DEF(AssetInfo);

    /// AssetInfo class
    class AssetInfo
    {
        CLASS_NO_COPY_MOVE(AssetInfo);

    public:
        /// virtual destructor
        virtual ~AssetInfo() = default;

        /// get path
        [[nodiscard]] virtual const std::filesystem::path &Path() const = 0;

        /// get UUID
        [[nodiscard]] virtual Lumen::UUID UUID() const = 0;

        /// get the asset type
        [[nodiscard]] virtual HashType Type() const = 0;

        /// whether the asset is a builtin asset, its UUID is in the reserved builtin range
        [[nodiscard]] bool Builtin() const { return UUIDIsBuiltin(UUID()); }

        /// serialize the asset info, it is stored in the Library entry of the asset, next to the asset itself
        void Serialize(Serialized::Type &out, bool packed) const;

        /// deserialize the fields of a serialized asset info, false when one is missing
        [[nodiscard]] static bool Deserialize(const Serialized::Type &in, bool packed, Lumen::UUID &uuid, std::filesystem::path &path, Hash &type);

    protected:
        /// default constructor
        explicit AssetInfo() = default;
    };
}
