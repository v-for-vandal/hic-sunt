#pragma once

#include <core/variables/numeric_variable.hpp>

namespace hs::scope {

template <typename BaseTypes = StdBaseTypes>
using NumericVariable = hs::variables::NumericVariable<BaseTypes>;

}  // namespace hs::scope
