//==============================================================================================================================================================================
/// \file
/// \brief     Json help include interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <nlohmann/json.hpp>

/// json type alias
using json = nlohmann::basic_json<nlohmann::ordered_map>;
