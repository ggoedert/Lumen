//==============================================================================================================================================================================
/// \file
/// \brief     SerializedData helper include interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <nlohmann/json.hpp>

/// serialized data type alias
using SerializedData = nlohmann::basic_json<nlohmann::ordered_map>;
