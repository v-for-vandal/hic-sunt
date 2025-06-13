#pragma once

#include <memory>

#include <core/utils/string_token.hpp>

namespace hs::region {

template<typename BaseTypes>
class Region;

template<typename BaseTypes>
using RegionPtr = std::shared_ptr<Region<BaseTypes>>;

}
