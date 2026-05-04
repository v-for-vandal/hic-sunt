#include "scope_change_set.hpp"

#include <gtest/gtest.h>

#include <core/scope/scope_ut.hpp>
#include <core/ruleset/variable_definition.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::scope {

using StdScope = Scope<StdBaseTypes>;
using StdScopePtr = ScopePtr<StdBaseTypes>;
using StdScopeChangeSet = ScopeChangeSet<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using StdVariableDefinitionsPtr =
    hs::ruleset::VariableDefinitionsPtr<StdBaseTypes>;

TEST(StdScopeChangeSet, EmptyAndClear) {
  auto scope = test::MakeSimpleScope();
  StdScopeChangeSet changes(scope);

  EXPECT_TRUE(changes.Empty());

  EXPECT_TRUE(changes.SetNumericModifier("numeric_var", "key", 1.0, 2.0));
  EXPECT_FALSE(changes.Empty());

  changes.Clear();
  EXPECT_TRUE(changes.Empty());
}

TEST(StdScopeChangeSet, RejectsEmptyModifierKey) {
  auto scope = test::MakeSimpleScope();
  StdScopeChangeSet changes(scope);

  auto result = changes.SetNumericModifier("numeric_var", "", 1.0, 2.0);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EMPTY_MODIFIER_KEY);
}

TEST(StdScopeChangeSet, RejectsIncorrectNumericVariableType) {
  auto scope = test::MakeSimpleScope();
  StdScopeChangeSet changes(scope);

  auto result = changes.SetNumericModifier("string_var", "key", 1.0, 2.0);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdScopeChangeSet, RejectsIncorrectStringVariableType) {
  auto scope = test::MakeSimpleScope();
  StdScopeChangeSet changes(scope);

  auto result = changes.SetStringModifier("numeric_var", "key", "value", 1.0);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdScopeChangeSet, AppliesNumericOperationsInInsertionOrder) {
  auto scope = test::MakeSimpleScope();
  StdScopeChangeSet changes(scope);

  ASSERT_TRUE(changes.SetNumericModifier("numeric_var", "key", 10.0, 0.0));
  ASSERT_TRUE(changes.ChangeNumericModifier("numeric_var", "key", 5.0, 1.0));
  ASSERT_TRUE(changes.SetNumericModifier("numeric_var", "key", 3.0, 0.0));
  ASSERT_TRUE(changes.ChangeNumericModifier("numeric_var", "key", 4.0, 0.0));

  ASSERT_TRUE(changes.Apply(42));

  auto value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, 7.0);

  auto modification_time = scope->GetModificationTime("numeric_var");
  ASSERT_TRUE(modification_time.has_value());
  EXPECT_EQ(*modification_time, 42u);
}

TEST(StdScopeChangeSet, AppliesStringOperations) {
  auto scope = test::MakeSimpleScope();
  StdScopeChangeSet changes(scope);

  ASSERT_TRUE(changes.SetStringModifier("string_var", "low", "first", 1.0));
  ASSERT_TRUE(changes.SetStringModifier("string_var", "high", "second", 2.0));

  ASSERT_TRUE(changes.Apply(17));

  auto value = scope->GetStringValue("string_var");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "second");

  auto modification_time = scope->GetModificationTime("string_var");
  ASSERT_TRUE(modification_time.has_value());
  EXPECT_EQ(*modification_time, 17u);
}

}  // namespace hs::scope
