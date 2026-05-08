#include <core/ruleset/variable_definition.hpp>

#include <gtest/gtest.h>

namespace hs::ruleset {

using StdVariableDefinitions = VariableDefinitions<StdBaseTypes>;
using VariableType = StdVariableDefinitions::VariableType;

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
  EXPECT_EQ(found.minimum, 10);
  EXPECT_EQ(found.maximum, 20);
}

}  // namespace hs::ruleset
