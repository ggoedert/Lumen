//==============================================================================================================================================================================
/// \file
/// \brief     SerializedData helper
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lSerializedData.h"
#include "lDefs.h"

using namespace Lumen;

const std::string Serialized::cPathToken = std::string("Path");
const std::string Serialized::cNameToken = std::string("Name");
const std::string Serialized::cScaleToken = std::string("Scale");

const std::string Serialized::cTransformToken = std::string("Transform");
const std::string Serialized::cTransformTokenPacked = Base64Encode(HashString(Serialized::cTransformToken.c_str()));
const std::string Serialized::cComponentsToken = std::string("Components");
const std::string Serialized::cComponentsTokenPacked = Base64Encode(HashString(Serialized::cComponentsToken.c_str()));
const std::string Serialized::cPropertiesToken = std::string("Properties");
const std::string Serialized::cPropertiesTokenPacked = Base64Encode(HashString(Serialized::cPropertiesToken.c_str()));
const std::string Serialized::cPositionToken = std::string("Position");
const std::string Serialized::cPositionTokenPacked = Base64Encode(HashString(Serialized::cPositionToken.c_str()));
const std::string Serialized::cRotationToken = std::string("Rotation");
const std::string Serialized::cRotationTokenPacked = Base64Encode(HashString(Serialized::cRotationToken.c_str()));
const std::string Serialized::cBackgroundColorToken = std::string("BackgroundColor");
const std::string Serialized::cBackgroundColorTokenPacked = Base64Encode(HashString(Serialized::cBackgroundColorToken.c_str()));
const std::string Serialized::cMeshTypeToken = std::string("Lumen::Mesh");
const std::string Serialized::cMeshTypeTokenPacked = Base64Encode(HashString(Serialized::cMeshTypeToken.c_str()));
const std::string Serialized::cTextureTypeToken = std::string("Lumen::Texture");
const std::string Serialized::cTextureTypeTokenPacked = Base64Encode(HashString(Serialized::cTextureTypeToken.c_str()));
const std::string Serialized::cMaterialTypeToken = std::string("Lumen::Material");
const std::string Serialized::cMaterialTypeTokenPacked = Base64Encode(HashString(Serialized::cMaterialTypeToken.c_str()));
