//==============================================================================================================================================================================
/// \file
/// \brief     AssetInfo
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssetInfo.h"

using namespace Lumen;

/// serialize the asset info
void AssetInfo::Serialize(Serialized::Type &out, bool packed) const
{
    out = packed ? Serialized::Type::array() : Serialized::Type::object();
    Serialized::SerializeUUID(out, packed, Serialized::cUUIDToken, Serialized::cUUIDTokenPacked, UUID());
    Serialized::SerializeValue(out, packed, Serialized::cPathToken, Serialized::cPathTokenPacked, Path().generic_string());
    Serialized::SerializeValue(out, packed, Serialized::cTypeToken, Serialized::cTypeTokenPacked, static_cast<Hash>(Type()));
}

/// deserialize the fields of a serialized asset info
bool AssetInfo::Deserialize(const Serialized::Type &in, bool packed, Lumen::UUID &uuid, std::filesystem::path &path, Hash &type)
{
    Lumen::UUID inUUID = UUID_INVALID;
    Serialized::Type inPath = {};
    Serialized::Type inType = {};
    if (!Serialized::DeserializeUUID(in, packed, Serialized::cUUIDToken, Serialized::cUUIDTokenPacked, inUUID) || (inUUID == UUID_INVALID) ||
        !Serialized::DeserializeValue(in, packed, Serialized::cPathToken, Serialized::cPathTokenPacked, inPath) || !inPath.is_string() ||
        !Serialized::DeserializeValue(in, packed, Serialized::cTypeToken, Serialized::cTypeTokenPacked, inType) || !inType.is_number_unsigned())
    {
        return false;
    }
    uuid = inUUID;
    path = inPath.get<std::string>();
    type = inType.get<Hash>();
    return true;
}
