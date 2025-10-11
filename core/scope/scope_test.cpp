#include "scope.hpp"

#include <gtest/gtest.h>

#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

namespace hs::scope {

using StdScope = Scope<StdBaseTypes>;
using StdScopePtr = ScopePtr<StdBaseTypes>;

TEST(StdScope, Create) {
  StdScope stack_scope;

  StdScopePtr ptr_scope;
}

TEST(StdScope, NumericVariable) {
  StdScope scope("test");

  scope.AddNumericModifier("some_var", "some_key", 1.0, 2.0);
  auto result = scope.GetNumericValue("some_var");
  EXPECT_EQ(result, 3.0);  // add=1.0 * (1 + mult=2.0)
}

TEST(StdScope, InheritanceParent) {
  StdScopePtr parent_scope("parent");
  StdScope scope("test");

  scope.SetParent(parent_scope);

  scope.AddNumericModifier("some_var", "some_key", 1.0, 1.0);
  parent_scope->AddNumericModifier("some_var", "other_key", 1.0, 1.0);
  auto result = scope.GetNumericValue("some_var");
  EXPECT_EQ(result, 6.0);  // add=(1+1) * (1 + mult=(1+1))
}

}  // namespace hs::scope
