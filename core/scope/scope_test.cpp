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

  EXPECT_TRUE(scope.SetNumericModifier("some_var", "some_key", 1.0, 2.0, 0));
  auto result = scope.GetNumericValue("some_var");
  EXPECT_EQ(result, 3.0);  // add=1.0 * (1 + mult=2.0)
}

TEST(StdScope, InheritanceParent) {
  StdScopePtr parent_scope("parent");
  StdScope scope("test");

  scope.SetParent(parent_scope);

  EXPECT_TRUE(scope.SetNumericModifier("some_var", "some_key", 1.0, 1.0, 0));
  EXPECT_TRUE(parent_scope->SetNumericModifier("some_var", "other_key", 1.0, 1.0));
  auto result = scope.GetNumericValue("some_var");
  EXPECT_EQ(result, 6.0);  // add=(1+1) * (1 + mult=(1+1))
}

}  // namespace hs::scope
