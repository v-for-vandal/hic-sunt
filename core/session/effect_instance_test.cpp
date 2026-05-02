#include "effect_instance.hpp"

#include <gtest/gtest.h>

#include <ruleset/effect.pb.h>

namespace hs::session {

using StdEffectDefinition = ruleset::EffectDefinition<StdBaseTypes>;
using StdEffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<StdBaseTypes>;
using StdEffectInstance = EffectInstance<StdBaseTypes>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;
using StdScopeChangeSet = scope::ScopeChangeSet<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;

namespace {

StdEffectDefinitionPtr MakeEffectDefinition(std::string id,
                                            std::string possible,
                                            std::string effect_code) {
  proto::ruleset::effect::Effect effect;
  effect.set_id(std::move(id));
  effect.mutable_possible()->set_lua(std::move(possible));
  effect.mutable_effect()->set_lua(std::move(effect_code));

  auto definition = std::make_shared<StdEffectDefinition>(std::move(effect));
  return StdEffectDefinitionPtr(
      std::static_pointer_cast<const StdEffectDefinition>(definition));
}

StdScopePtr MakeScope() {
  auto definitions = std::make_shared<StdVariableDefinitions>();
  definitions->AddNumericDefinition("numeric_var", {});
  definitions->AddStringDefinition("string_var", {});

  StdScopePtr scope("test_scope");
  scope->SetVariableDefinitions(definitions);
  return scope;
}

}  // namespace

TEST(StdEffectInstance, ExposesDefinitionIdentity) {
  auto definition = MakeEffectDefinition("effect.id", "return true", "return");

  StdEffectInstance instance(definition);

  EXPECT_EQ(instance.GetId(), "effect.id");
  EXPECT_EQ(instance.GetDefinition()->GetId(), "effect.id");
}

TEST(StdEffectInstance, CheckPossibleReturnsBoolean) {
  auto definition = MakeEffectDefinition("effect.id", "return true", "return");

  StdEffectInstance instance(definition);
  auto scope = MakeScope();
  StdScopeChangeSet changes(scope);

  auto result = instance.CheckPossible(changes, 10000);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(*result);
}

TEST(StdEffectInstance, CheckPossibleRejectsNonBooleanResult) {
  auto definition = MakeEffectDefinition("effect.id", "return 1", "return");

  StdEffectInstance instance(definition);
  auto scope = MakeScope();
  StdScopeChangeSet changes(scope);

  auto result = instance.CheckPossible(changes, 10000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
}

TEST(StdEffectInstance, ExecuteAppliesChangesToTargetChangeSet) {
  auto definition = MakeEffectDefinition(
      "effect.id",
      "return true",
      "target:set_numeric_modifier('numeric_var', 2.0, 0.5) "
      "target:set_string_modifier('string_var', 'value', 3.0)");

  StdEffectInstance instance(definition);
  auto scope = MakeScope();
  StdScopeChangeSet changes(scope);

  ASSERT_TRUE(instance.Execute(changes, 10000));
  ASSERT_TRUE(changes.Apply(17));

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 3.0);

  auto string_value = scope->GetStringValue("string_var");
  ASSERT_TRUE(string_value.has_value());
  EXPECT_EQ(*string_value, "value");
}

TEST(StdEffectInstance, ExecuteUsesEffectIdAsModifierKey) {
  auto definition = MakeEffectDefinition(
      "effect.id",
      "return true",
      "target:set_numeric_modifier('numeric_var', 2.0, 0.0)");

  StdEffectInstance instance(definition);
  auto scope = MakeScope();
  StdScopeChangeSet changes(scope);

  ASSERT_TRUE(instance.Execute(changes, 10000));
  ASSERT_TRUE(changes.Apply(17));

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 2.0);
}

TEST(StdEffectInstance, ExecuteReturnsRuntimeErrorForLuaFailure) {
  auto definition = MakeEffectDefinition(
      "effect.id",
      "return true",
      "error('boom')");

  StdEffectInstance instance(definition);
  auto scope = MakeScope();
  StdScopeChangeSet changes(scope);

  auto result = instance.Execute(changes, 10000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
}

TEST(StdEffectInstance, ExecuteReturnsOperationLimitError) {
  auto definition = MakeEffectDefinition(
      "effect.id",
      "return true",
      "while true do end");

  StdEffectInstance instance(definition);
  auto scope = MakeScope();
  StdScopeChangeSet changes(scope);

  auto result = instance.Execute(changes, 1000);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_EFFECT_LUA_OPERATION_LIMIT_EXCEEDED);
}

}  // namespace hs::session
