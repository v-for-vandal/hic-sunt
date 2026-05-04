#include "effect_instance.hpp"

#include <gtest/gtest.h>

#include <ruleset/effect.pb.h>

#include <core/ruleset/ruleset_ut.hpp>
#include <core/scope/scope_ut.hpp>

namespace hs::session {

using StdEffectDefinition = ruleset::EffectDefinition<StdBaseTypes>;
using StdEffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<StdBaseTypes>;
using StdEffectInstance = EffectInstance<StdBaseTypes>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;

TEST(StdEffectInstance, ExposesDefinitionIdentity) {
  auto definition = hs::ruleset::test::MakeEffectDefinition("effect.id", "return true", "return");

  StdEffectInstance instance(definition);

  EXPECT_EQ(instance.GetId(), "effect.id");
  EXPECT_EQ(instance.GetDefinition()->GetId(), "effect.id");
}

TEST(StdEffectInstance, CheckPossibleReturnsBoolean) {
  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id", "return VAR(numeric_var) == 2.0", "return");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto result = instance.CheckPossible(scope, 10000);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(*result);
}

TEST(StdEffectInstance, CheckPossibleRejectsNonBooleanResult) {
  auto definition = hs::ruleset::test::MakeEffectDefinition("effect.id", "return 1", "return");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto result = instance.CheckPossible(scope, 10000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
}

TEST(StdEffectInstance, ExecuteReturnsChangeSetWithAppliedChanges) {
  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id",
      "return true",
      "target:set_numeric_modifier('numeric_var', VAR(numeric_var), 0.5) "
      "target:set_string_modifier('string_var', VAR(string_var), 3.0)");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto execute_result = instance.Execute(scope, 10000);
  ASSERT_TRUE(execute_result.has_value());
  ASSERT_EQ(execute_result->size(), 1u);
  ASSERT_TRUE((*execute_result)[0].Apply(17));

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 6.0);

  auto string_value = scope->GetStringValue("string_var");
  ASSERT_TRUE(string_value.has_value());
  EXPECT_EQ(*string_value, "value");
}

TEST(StdEffectInstance, ExecuteUsesEffectIdAsModifierKey) {
  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id",
      "return true",
      "target:set_numeric_modifier('numeric_var', 2.0, 0.0)");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto execute_result = instance.Execute(scope, 10000);
  ASSERT_TRUE(execute_result.has_value());
  ASSERT_EQ(execute_result->size(), 1u);
  ASSERT_TRUE((*execute_result)[0].Apply(17));

  struct NumericExplanation {
    std::string scope_id;
    std::string variable;
    std::string modifier;
    double add{0};
    double mult{0};
  };

  std::vector<NumericExplanation> explanations;
  scope->ExplainNumericVariable(
      "numeric_var",
      [&explanations](const auto& scope_id, const auto& variable,
                      const auto& modifier, auto add, auto mult) {
        explanations.push_back(NumericExplanation{
            .scope_id = scope_id,
            .variable = variable,
            .modifier = modifier,
            .add = add,
            .mult = mult,
        });
      });

  ASSERT_EQ(explanations.size(), 2u);
  const auto fit = std::ranges::find_if(explanations, [](const auto& item) {
    return item.modifier == "effect.id";
  });
  ASSERT_NE(fit, explanations.end());
  EXPECT_EQ(fit->add, 2.0);
  EXPECT_EQ(fit->mult, 0.0);
}

TEST(StdEffectInstance, ExecuteReturnsRuntimeErrorForLuaFailure) {
  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id",
      "return true",
      "error('boom')");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto result = instance.Execute(scope, 10000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
}

TEST(StdEffectInstance, ExecuteReturnsOperationLimitError) {
  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id",
      "return true",
      "while true do end");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto result = instance.Execute(scope, 1000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_OPERATION_LIMIT_EXCEEDED);
}

}  // namespace hs::session
