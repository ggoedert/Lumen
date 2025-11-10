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

        /// name token
        extern const std::string cNameToken;

        /// path token
        extern const std::string cPathToken;

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

        /// serialized asset
        inline void SerializeAsset(Type &out, bool packed, const std::string &key, const Hash &keyPacked, const std::string &name, const std::string &path)
        {
            if (packed)
            {
                Serialized::Type assetArray = Serialized::Type::array();
                assetArray.push_back(name);
                if (!path.empty())
                {
                    assetArray.push_back(path);
                }
                out.push_back(keyPacked);
                out.push_back(assetArray);
            }
            else
            {
                Serialized::Type assetObj = Serialized::Type::object();
                assetObj[Serialized::cNameToken] = name;
                if (!path.empty())
                {
                    assetObj[Serialized::cPathToken] = path;
                }
                out[key] = assetObj;
            }
        }

        /// deserialize asset
        inline void DeserializeAsset(const Type &in, bool packed, std::string keyToken, Hash keyTokenPacked, std::string &name, std::string &path)
        {
            name = "";
            path = "";
            if (packed)
            {
                for (size_t i = 1; i < in.size(); i += 2)
                {
                    const auto &obj = in[i - 1];
                    if (obj.is_number_unsigned() && obj.get<Hash>() == keyTokenPacked)
                    {
                        auto obj = in[1];
                        name = obj[0].get<std::string>();
                        if (obj.size() > 1)
                        {
                            path = obj[1].get<std::string>();;
                        }
                    }
                }
            }
            else
            {
                if (in.contains(keyToken))
                {
                    auto obj = in[keyToken];
                    if (obj.contains(Serialized::cNameToken))
                    {
                        name = obj[Serialized::cNameToken].get<std::string>();
                    }
                    if (obj.contains(Serialized::cPathToken))
                    {
                        path = obj[Serialized::cPathToken].get<std::string>();
                    }
                }
            }
        }
    }
}
