#include "scope_change_set.hpp"

#include <gtest/gtest.h>

#include <core/ruleset/variable_definition.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::scope {

using StdScope = Scope<StdBaseTypes>;
using StdScopePtr = ScopePtr<StdBaseTypes>;
using StdScopeChangeSet = ScopeChangeSet<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using StdVariableDefinitionsPtr =
    hs::ruleset::VariableDefinitionsPtr<StdBaseTypes>;

namespace {

StdVariableDefinitionsPtr MakeVariableDefinitions() {
  auto mutable_definitions = std::make_shared<StdVariableDefinitions>();
  mutable_definitions->AddNumericDefinition("numeric_var", {});
  mutable_definitions->AddStringDefinition("string_var", {});

  return StdVariableDefinitionsPtr(
      std::static_pointer_cast<const StdVariableDefinitions>(
          mutable_definitions));
}

StdScopePtr MakeScopeWithDefinitions() {
  StdScopePtr scope("test_scope");
  scope->SetVariableDefinitions(MakeVariableDefinitions());
  return scope;
}

}  // namespace

TEST(StdScopeChangeSet, EmptyAndClear) {
  auto scope = MakeScopeWithDefinitions();
  StdScopeChangeSet changes(scope);

  EXPECT_TRUE(changes.Empty());

  EXPECT_TRUE(changes.SetNumericModifier("numeric_var", "key", 1.0, 2.0));
  EXPECT_FALSE(changes.Empty());

  changes.Clear();
  EXPECT_TRUE(changes.Empty());
}

TEST(StdScopeChangeSet, RejectsEmptyModifierKey) {
  auto scope = MakeScopeWithDefinitions();
  StdScopeChangeSet changes(scope);

  auto result = changes.SetNumericModifier("numeric_var", "", 1.0, 2.0);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EMPTY_MODIFIER_KEY);
}

TEST(StdScopeChangeSet, RejectsIncorrectNumericVariableType) {
  auto scope = MakeScopeWithDefinitions();
  StdScopeChangeSet changes(scope);

  auto result = changes.SetNumericModifier("string_var", "key", 1.0, 2.0);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdScopeChangeSet, RejectsIncorrectStringVariableType) {
  auto scope = MakeScopeWithDefinitions();
  StdScopeChangeSet changes(scope);

  auto result = changes.SetStringModifier("numeric_var", "key", "value", 1.0);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdScopeChangeSet, AppliesNumericOperationsInInsertionOrder) {
  auto scope = MakeScopeWithDefinitions();
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
  auto scope = MakeScopeWithDefinitions();
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
