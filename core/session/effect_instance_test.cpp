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

TEST(StdEffectInstance, RejectsBrokenDefinition) {
  auto definition =
      hs::ruleset::test::MakeEffectDefinition("effect.id", "this is not valid lua", "return");

  EXPECT_THROW((StdEffectInstance(definition)), std::system_error);
}

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

TEST(StdEffectInstance, CheckPossibleReturnsTrueWhenPossibleCodeIsEmpty) {
  auto definition = hs::ruleset::test::MakeEffectDefinition("effect.id", "", "return");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto result = instance.CheckPossible(scope, 10000);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(*result);
}

TEST(StdEffectInstance, CheckPossibleReturnsTrueWhenPossibleCodeIsEmptyAndScopeHasNoDependencies) {
  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id", "", "target:set_numeric_modifier('numeric_var', 1.0, 0.0)");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_REGION);

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

TEST(StdEffectInstance, WildcardParameterizedVarBindsLuaDictionary) {
  auto mutable_definitions = std::make_shared<StdVariableDefinitions>();
  ASSERT_TRUE(mutable_definitions
                  ->AddParameterizedNumericDefinition(
                      "job/{job}/consumes/{resource}",
                      {
                          hs::ruleset::OpenParameterDomain<StdBaseTypes>("job"),
                          hs::ruleset::OpenParameterDomain<StdBaseTypes>("resource"),
                      },
                      {})
                  .has_value());
  hs::ruleset::VariableDefinitionsConstPtr<StdBaseTypes> definitions{
      std::static_pointer_cast<const StdVariableDefinitions>(mutable_definitions)};

  StdScopePtr scope("scope", types::ScopeType::SCOPE_TYPE_WORLD);
  scope->SetVariableDefinitions(definitions);
  ASSERT_TRUE(scope->SetNumericModifier("job/@job.farmer/consumes/@resource.wood", "seed", 2, 0));
  ASSERT_TRUE(scope->SetNumericModifier("job/@job.baker/consumes/@resource.wood", "seed", 5, 0));
  ASSERT_TRUE(scope->SetNumericModifier("job/@job.farmer/consumes/@resource.food", "seed", 100, 0));

  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id",
      "local total = 0; for id,value in pairs(VAR(job/@*/consumes/@resource.wood)) do total = "
      "total + value end; return total == 7",
      "return");

  StdEffectInstance instance(definition);
  auto result = instance.CheckPossible(scope, 10000);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(*result);
}

TEST(StdEffectInstance, ExecuteReturnsChangeSetWithAppliedChanges) {
  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id", "return true",
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
      "effect.id", "return true", "target:set_numeric_modifier('numeric_var', 2.0, 0.0)");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto execute_result = instance.Execute(scope, 10000);
  ASSERT_TRUE(execute_result.has_value());
  ASSERT_EQ(execute_result->size(), 1u);
  ASSERT_TRUE((*execute_result)[0].Apply(17));

  std::vector<scope::test::NumericExplanation> explanations;
  scope->ExplainNumericVariable("numeric_var",
                                [&explanations](const auto& scope_id, const auto& variable,
                                                const auto& modifier, auto add, auto mult) {
                                  explanations.push_back(scope::test::NumericExplanation{
                                      .scope_id = scope_id,
                                      .variable = variable,
                                      .modifier = modifier,
                                      .add = add,
                                      .mult = mult,
                                  });
                                });

  ASSERT_EQ(explanations.size(), 2u);
  const auto fit = std::ranges::find_if(
      explanations, [](const auto& item) { return item.modifier == "effect.id"; });
  ASSERT_NE(fit, explanations.end());
  EXPECT_EQ(fit->add, 2.0);
  EXPECT_EQ(fit->mult, 0.0);
}

TEST(StdEffectInstance, ExecuteReturnsRuntimeErrorForLuaFailure) {
  auto definition =
      hs::ruleset::test::MakeEffectDefinition("effect.id", "return true", "error('boom')");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto result = instance.Execute(scope, 10000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
}

TEST(StdEffectInstance, ExecuteReturnsOperationLimitError) {
  auto definition =
      hs::ruleset::test::MakeEffectDefinition("effect.id", "return true", "while true do end");

  StdEffectInstance instance(definition);
  auto scope = hs::scope::test::MakeSeededScope();

  auto result = instance.Execute(scope, 1000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_OPERATION_LIMIT_EXCEEDED);
}

}  // namespace hs::session
