#pragma once

#include <core/utils/shared_ptr.hpp>
#include <core/utils/string_token.hpp>

namespace hs::terra {

template <typename BaseTypes>
class Plane;

template <typename BaseTypes>
using PlanePtr = utils::SharedPtr<Plane<BaseTypes>>;

}  // namespace hs::terra
