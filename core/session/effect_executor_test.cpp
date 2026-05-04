#include "effect_executor.hpp"

#include <gtest/gtest.h>

#include <ruleset/effect.pb.h>

#include <memory>
#include <vector>

#include <core/ruleset/variable_definition.hpp>

namespace hs::session {

using StdSession = Session<StdBaseTypes>;
using StdEffectExecutor = EffectExecutor<StdBaseTypes>;
using StdEffectDefinition = ruleset::EffectDefinition<StdBaseTypes>;
using StdEffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<StdBaseTypes>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using ScopeType = types::ScopeType;

namespace {

StdEffectDefinitionPtr MakeEffectDefinition(std::string id, ScopeType scope_type,
                                            std::string possible,
                                            std::string effect_code) {
  proto::ruleset::effect::Effect effect;
  effect.set_id(std::move(id));
  effect.set_scope_type(scope_type);
  effect.mutable_possible()->set_lua(std::move(possible));
  effect.mutable_effect()->set_lua(std::move(effect_code));

  auto definition = std::make_shared<StdEffectDefinition>(std::move(effect));
  return StdEffectDefinitionPtr(
      std::static_pointer_cast<const StdEffectDefinition>(definition));
}

StdScopePtr MakeScope(const std::string& id, ScopeType type) {
  auto definitions = std::make_shared<StdVariableDefinitions>();
  definitions->AddNumericDefinition("numeric_var", {});
  definitions->AddStringDefinition("string_var", {});

  StdScopePtr scope(id, type);
  scope->SetVariableDefinitions(definitions);
  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 2.0, 0.0));
  EXPECT_TRUE(scope->SetStringModifier("string_var", "seed", "value", 1.0));
  return scope;
}

}  // namespace

TEST(StdEffectExecutor, ExecutesQueuedEffectsAndAppliesChanges) {
  StdSession session;
  auto scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION,
      "return VAR(numeric_var) >= 0",
      "target:set_numeric_modifier('numeric_var', 4.0, 0.0)");
  session.GetEffects().push_back(std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  ASSERT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 10));

  StdEffectExecutor executor;
  executor.Execute(session, 10);

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 7.0);
}

TEST(StdEffectExecutor, SkipsEffectWhenDependenciesAreOlderThanCurrentTime) {
  StdSession session;
  auto scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION,
      "return VAR(numeric_var) >= 0",
      "target:set_numeric_modifier('numeric_var', 4.0, 0.0)");
  session.GetEffects().push_back(std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  ASSERT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 9));

  StdEffectExecutor executor;
  executor.Execute(session, 10);

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 3.0);
}

TEST(StdEffectExecutor, SkipsEffectWhenPossibleReturnsFalse) {
  StdSession session;
  auto scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION,
      "return false",
      "target:set_numeric_modifier('numeric_var', 4.0, 0.0)");
  session.GetEffects().push_back(std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  ASSERT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 10));

  StdEffectExecutor executor;
  executor.Execute(session, 10);

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 3.0);
}

}  // namespace hs::session
