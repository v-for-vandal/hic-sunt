#pragma once

#include <core/utils/shared_ptr.hpp>
#include <core/utils/string_token.hpp>

namespace hs::terra {

class Plane;

using PlanePtr = utils::SharedPtr<Plane>;

using PlaneId = utils::StringToken;
using PlaneIdCRef = utils::StringTokenCRef;

}

