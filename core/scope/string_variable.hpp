#pragma once

#include <core/variables/string_variable.hpp>

namespace hs::scope {

template <typename BaseTypes = StdBaseTypes>
using StringVariable = hs::variables::StringVariable<BaseTypes>;

}  // namespace hs::scope
