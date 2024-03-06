#pragma once

#include <memory>

#include <core/utils/string_token.hpp>

namespace hs::region {

class Region;

using RegionPtr = std::shared_ptr<Region>;

using RegionId = utils::StringToken;
using RegionIdCRef = utils::StringTokenCRef;

}
