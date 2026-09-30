#pragma once

#include <core/variables/variable_base.hpp>

namespace hs::scope {

template <typename BaseTypes = StdBaseTypes>
using VariableBase = hs::variables::VariableBase<BaseTypes>;

}  // namespace hs::scope
