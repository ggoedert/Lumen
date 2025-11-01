//==============================================================================================================================================================================
/// \file
/// \brief     SerializedData helper interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <nlohmann/json.hpp>

/// Lumen namespace
namespace Lumen
{
    /// Serialized namespace
    namespace Serialized
    {
        /// serialized type alias
        using Type = nlohmann::basic_json<nlohmann::ordered_map>;

        // tokens too small to Base64Encode

        /// path token
        extern const std::string cPathToken;

        /// name token
        extern const std::string cNameToken;

        /// scale token
        extern const std::string cScaleToken;

        // normal tokens

        /// transform token
        extern const std::string cTransformToken;

        /// transform token packed
        extern const std::string cTransformTokenPacked;

        /// components token
        extern const std::string cComponentsToken;

        /// components token packed
        extern const std::string cComponentsTokenPacked;

        /// properties token
        extern const std::string cPropertiesToken;

        /// properties token packed
        extern const std::string cPropertiesTokenPacked;

        /// position token
        extern const std::string cPositionToken;

        /// position token packed
        extern const std::string cPositionTokenPacked;

        /// rotation token
        extern const std::string cRotationToken;

        /// rotation token packed
        extern const std::string cRotationTokenPacked;

        /// background color token
        extern const std::string cBackgroundColorToken;

        /// background color token packed
        extern const std::string cBackgroundColorTokenPacked;

        /// mesh type token
        extern const std::string cMeshTypeToken;

        /// mesh type token packed
        extern const std::string cMeshTypeTokenPacked;

        /// texture type token
        extern const std::string cTextureTypeToken;

        /// texture type token packed
        extern const std::string cTextureTypeTokenPacked;

        /// material type token
        extern const std::string cMaterialTypeToken;

        /// texture type token packed
        extern const std::string cMaterialTypeTokenPacked;
    }
}
