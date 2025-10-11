#pragma once

#include <core/utils/string_token.hpp>
#include <memory>

namespace hs::region {

template <typename BaseTypes>
class Region;

template <typename BaseTypes>
using RegionPtr = std::shared_ptr<Region<BaseTypes>>;

}  // namespace hs::region
