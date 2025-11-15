//==============================================================================================================================================================================
/// \file
/// \brief     SerializedData helper interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

#include <nlohmann/json.hpp>
#include <variant>

/// Lumen namespace
namespace Lumen
{
    /// Serialized namespace
    namespace Serialized
    {
        /// serialized type alias
        using Type = nlohmann::basic_json<nlohmann::ordered_map>;

        /// transform token
        extern const std::string cTransformToken;

        /// transform token packed
        extern const Hash cTransformTokenPacked;

        /// components token
        extern const std::string cComponentsToken;

        /// components token packed
        extern const Hash cComponentsTokenPacked;

        /// position token
        extern const std::string cPositionToken;

        /// position token packed
        extern const Hash cPositionTokenPacked;

        /// rotation token
        extern const std::string cRotationToken;

        /// rotation token packed
        extern const Hash cRotationTokenPacked;

        /// scale token
        extern const std::string cScaleToken;

        /// scale token packed
        extern const Hash cScaleTokenPacked;

        /// background color token
        extern const std::string cBackgroundColorToken;

        /// background color token packed
        extern const Hash cBackgroundColorTokenPacked;

        /// mesh type token
        extern const std::string cMeshTypeToken;

        /// mesh type token packed
        extern const Hash cMeshTypeTokenPacked;

        /// material type token
        extern const std::string cMaterialTypeToken;

        /// material type token packed
        extern const Hash cMaterialTypeTokenPacked;

        /// properties token
        extern const std::string cPropertiesToken;

        /// properties token packed
        extern const Hash cPropertiesTokenPacked;

        /// texture type token
        extern const std::string cTextureTypeToken;

        /// texture type token packed
        extern const Hash cTextureTypeTokenPacked;

        /// shader type token
        extern const std::string cShaderTypeToken;

        /// shader type token packed
        extern const Hash cShaderTypeTokenPacked;

        /// serialized value
        inline void SerializeValue(Type &out, bool packed, const std::string &key, const Hash &keyPacked, const Type &value)
        {
            if (packed)
            {
                out.push_back(keyPacked);
                out.push_back(value);
            }
            else
            {
                out[key] = value;
            }
        }

        /// deserialize value
        inline bool DeserializeValue(const Type &in, bool packed, std::string keyToken, Hash keyTokenPacked, Type &value)
        {
            if (packed)
            {
                for (size_t i = 1; i < in.size(); i += 2)
                {
                    const auto &obj = in[i - 1];
                    if (obj.is_number_unsigned() && obj.get<Hash>() == keyTokenPacked)
                    {
                        value = in[i];
                        return true;
                    }
                }
            }
            else
            {
                if (in.contains(keyToken))
                {
                    value = in[keyToken];
                    return true;
                }
            }
            return false;
        }
    }
}
