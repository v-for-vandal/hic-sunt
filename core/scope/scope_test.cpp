#include "scope.hpp"

#include <gtest/gtest.h>

#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>
#include <core/scope/scope_ut.hpp>

namespace hs::scope {

using StdScope = Scope<StdBaseTypes>;
using StdScopePtr = ScopePtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using StdVariableDefinitionsPtr =
    hs::ruleset::VariableDefinitionsPtr<StdBaseTypes>;


TEST(StdScope, Create) {
  StdScope stack_scope;

  StdScopePtr ptr_scope;
}

TEST(StdScope, NumericVariable) {
  auto scope = test::MakeSimpleScope();

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "some_key", 1.0, 2.0, 0));
  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 3.0);  // add=1.0 * (1 + mult=2.0)
}

TEST(StdScope, InheritanceParent) {
  StdScopePtr parent_scope = test::MakeSimpleScope();
  StdScopePtr scope("test");

  scope->SetParent(parent_scope);

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "some_key", 1.0, 1.0, 0));
  EXPECT_TRUE(parent_scope->SetNumericModifier("numeric_var", "other_key", 1.0, 1.0));
  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 6.0);  // add=(1+1) * (1 + mult=(1+1))
}

}  // namespace hs::scope
