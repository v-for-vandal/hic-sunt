#include <gtest/gtest.h>

#include <core/ruleset/variable_definition.hpp>
#include <core/types/scope_type.hpp>

namespace hs::ruleset {

using StdVariableDefinitions = VariableDefinitions<StdBaseTypes>;
using VariableType = StdVariableDefinitions::VariableType;
using ParsedVariableDefinition = StdVariableDefinitions::ParsedVariableDefinition;

TEST(StdVariableDefinitions, ReportsMissingVariableType) {
  StdVariableDefinitions definitions;

  EXPECT_FALSE(definitions.IsVariable("missing"));
  EXPECT_EQ(definitions.GetVariableType("missing"), VariableType::kMissing);
}

TEST(StdVariableDefinitions, ReportsExistingVariableType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddNumericDefinition("numeric_var", {}));
  ASSERT_TRUE(definitions.AddStringDefinition("string_var", {}));

  EXPECT_TRUE(definitions.IsVariable("numeric_var"));
  EXPECT_TRUE(definitions.IsVariable("string_var"));
  EXPECT_EQ(definitions.GetVariableType("numeric_var"), VariableType::kNumeric);
  EXPECT_EQ(definitions.GetVariableType("string_var"), VariableType::kString);
}

TEST(StdVariableDefinitions, RejectsRedefinitionWithDifferentType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddNumericDefinition("shared_var", {}));

  auto result = definitions.AddStringDefinition("shared_var", {});
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdVariableDefinitions, AllowsRedefinitionWithSameType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddNumericDefinition("numeric_var", {}));

  NumericVariableDefinition<StdBaseTypes> replacement;
  replacement.minimum = 10;
  replacement.maximum = 20;

  auto result = definitions.AddNumericDefinition("numeric_var", replacement);
  ASSERT_TRUE(result.has_value());

  auto found = definitions.FindNumericVariable("numeric_var");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->minimum, 10);
  EXPECT_EQ(found->maximum, 20);
}

TEST(StdVariableDefinitions, MissingLookupsReturnUnexpected) {
  StdVariableDefinitions definitions;

  auto numeric = definitions.FindNumericVariable("missing_numeric");
  ASSERT_FALSE(numeric.has_value());
  EXPECT_EQ(numeric.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);

  auto string_ = definitions.FindStringVariable("missing_string");
  ASSERT_FALSE(string_.has_value());
  EXPECT_EQ(string_.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);
}

TEST(StdVariableDefinitions, ParseStringVariableDefaultsAllowedScopesToAllScopeTypes) {
  proto::ruleset::Variable variable;
  variable.set_id("var.one");
  variable.mutable_string();

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto *definition = std::get_if<StringVariableDefinition<StdBaseTypes>>(&parsed);
  ASSERT_NE(definition, nullptr);

  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_PLANE]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_REGION]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CELL]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_PLAYER]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT_CLASS]);
}

TEST(StdVariableDefinitions, ParseNumericVariableLoadsAllowedScopesFromFilter) {
  proto::ruleset::Variable variable;
  variable.set_id("var.one");
  variable.mutable_numeric();
  variable.mutable_allowed_scopes()->add_scope_type_sets(
      proto::types::ScopeTypeSet::SCOPE_TYPE_SET_GEO);
  variable.mutable_allowed_scopes()->add_scope_types(
      proto::types::ScopeType::SCOPE_TYPE_CITY);

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto *definition = std::get_if<NumericVariableDefinition<StdBaseTypes>>(&parsed);
  ASSERT_NE(definition, nullptr);

  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_PLANE]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_REGION]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CELL]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);

  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_PLAYER]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT_CLASS]);
}

TEST(StdVariableDefinitions, ParseBooleanVariableAsNumericRange) {
  proto::ruleset::Variable variable;
  variable.set_id("var.bool");
  variable.mutable_boolean();

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto *definition = std::get_if<NumericVariableDefinition<StdBaseTypes>>(&parsed);
  ASSERT_NE(definition, nullptr);
  EXPECT_EQ(definition->minimum, 0);
  EXPECT_EQ(definition->maximum, 1);
}

TEST(StdVariableDefinitions, ParseVariableWithoutTypeReturnsError) {
  proto::ruleset::Variable variable;
  variable.set_id("var.invalid");

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto *error = std::get_if<ErrorCode>(&parsed);
  ASSERT_NE(error, nullptr);
  EXPECT_EQ(*error, ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

}  // namespace hs::ruleset
