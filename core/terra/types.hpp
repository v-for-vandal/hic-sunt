#pragma once

#include <core/utils/non_null_ptr.hpp>
#include <core/utils/shared_ptr.hpp>
#include <core/utils/string_token.hpp>

namespace hs::terra {

template <typename BaseTypes>
class Plane;

template <typename BaseTypes>
class Civilization;

template <typename BaseTypes>
using PlanePtr = utils::SharedPtr<Plane<BaseTypes>>;

template <typename BaseTypes>
using CivilizationPtr = utils::NonNullSharedPtr<Civilization<BaseTypes>>;

}  // namespace hs::terra
