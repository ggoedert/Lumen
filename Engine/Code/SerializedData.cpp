//==============================================================================================================================================================================
/// \file
/// \brief     SerializedData helper
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lSerializedData.h"

using namespace Lumen;

const std::string Serialized::cTransformToken = std::string("Transform");
const Hash        Serialized::cTransformTokenPacked = HashString(Serialized::cTransformToken.c_str());

const std::string Serialized::cComponentsToken = std::string("Components");
const Hash        Serialized::cComponentsTokenPacked = HashString(Serialized::cComponentsToken.c_str());

const std::string Serialized::cPositionToken = std::string("Position");
const Hash        Serialized::cPositionTokenPacked = HashString(Serialized::cPositionToken.c_str());
const std::string Serialized::cRotationToken = std::string("Rotation");
const Hash        Serialized::cRotationTokenPacked = HashString(Serialized::cRotationToken.c_str());
const std::string Serialized::cScaleToken = std::string("Scale");
const Hash        Serialized::cScaleTokenPacked = HashString(Serialized::cScaleToken.c_str());

const std::string Serialized::cBackgroundColorToken = std::string("BackgroundColor");
const Hash        Serialized::cBackgroundColorTokenPacked = HashString(Serialized::cBackgroundColorToken.c_str());

const std::string Serialized::cMeshTypeToken = std::string("Lumen::Mesh");
const Hash        Serialized::cMeshTypeTokenPacked = HashString(Serialized::cMeshTypeToken.c_str());

const std::string Serialized::cMaterialTypeToken = std::string("Lumen::Material");
const Hash        Serialized::cMaterialTypeTokenPacked = HashString(Serialized::cMaterialTypeToken.c_str());
const std::string Serialized::cPropertiesToken = std::string("Properties");
const Hash        Serialized::cPropertiesTokenPacked = HashString(Serialized::cPropertiesToken.c_str());
const std::string Serialized::cTextureTypeToken = std::string("Lumen::Texture");
const Hash        Serialized::cTextureTypeTokenPacked = HashString(Serialized::cTextureTypeToken.c_str());

const std::string Serialized::cShaderTypeToken = std::string("Lumen::Shader");
const Hash        Serialized::cShaderTypeTokenPacked = HashString(Serialized::cShaderTypeToken.c_str());
