#include "scope.hpp"

#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

#include <gtest/gtest.h>

namespace hs::scope {

using StdScope = Scope<StdBaseTypes>;
using StdScopePtr = ScopePtr<StdBaseTypes>;

TEST(StdScope, Create) {
    StdScope stack_scope;

    StdScopePtr ptr_scope;
}

}
