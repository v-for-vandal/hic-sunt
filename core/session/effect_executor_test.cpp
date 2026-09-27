#include "effect_executor.hpp"

#include <gtest/gtest.h>
#include <ruleset/effect.pb.h>

#include <core/ruleset/ruleset_ut.hpp>
#include <core/ruleset/variable_definition.hpp>
#include <core/scope/scope_ut.hpp>
#include <memory>
#include <vector>

namespace hs::session {

namespace {

proto::ruleset::effect::Effect MakeEffectWithSelector(std::string id, types::ScopeType scope_type,
                                                      std::string possible, std::string effect_code,
                                                      std::string selector_class = {},
                                                      std::string selector_scope_id = {}) {
  proto::ruleset::effect::Effect effect;
  effect.set_id(std::move(id));
  effect.set_scope_type(scope_type);
  effect.mutable_possible()->set_lua(std::move(possible));
  effect.mutable_effect()->set_lua(std::move(effect_code));
  if (!selector_class.empty()) {
    effect.mutable_selector()->set_class_(std::move(selector_class));
  }
  if (!selector_scope_id.empty()) {
    effect.mutable_selector()->set_scope_id(std::move(selector_scope_id));
  }
  return effect;
}

}  // namespace

using StdSession = Session<StdBaseTypes>;
using StdEffectExecutor = EffectExecutor<StdBaseTypes>;
using StdEffectDefinition = ruleset::EffectDefinition<StdBaseTypes>;
using StdEffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<StdBaseTypes>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using ScopeType = types::ScopeType;

TEST(StdEffectExecutor, ExecutesQueuedEffectsAndAppliesChanges) {
  StdSession session;
  auto scope = hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION, "return VAR(numeric_var) >= 0",
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
  auto scope = hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION, "return VAR(numeric_var) >= 0",
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
  auto scope = hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION, "return false",
      "target:set_numeric_modifier('numeric_var', 4.0, 0.0)");
  session.GetEffects().push_back(std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  ASSERT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 10));

  StdEffectExecutor executor;
  executor.Execute(session, 10);

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 3.0);
}

TEST(StdEffectExecutor, HandlingRuntimeError) {
  StdSession session;
  auto scope = hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = hs::ruleset::test::MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION, "",
      "a = VAR(numeric_var); target:set_numeric_modifier('numeric_var', "
      "'break', 4.0, 0.0, 0.0, 'no', 55)");
  session.GetEffects().push_back(std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  ASSERT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 10));

  StdEffectExecutor executor;
  executor.Execute(session, 10);

  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 3.0);
}

TEST(StdEffectExecutor, AppliesEffectOnlyToScopesMatchingSelectorClass) {
  StdSession session;
  auto matching_scope =
      hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION, "matching.region");
  auto non_matching_scope =
      hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION, "non_matching.region");
  ASSERT_TRUE(session.AddScope(matching_scope));
  ASSERT_TRUE(session.AddScope(non_matching_scope));

  ASSERT_TRUE(matching_scope->SetStringModifier("core.class", "seed", "forest", 1.0, 10));
  ASSERT_TRUE(non_matching_scope->SetStringModifier("core.class", "seed", "desert", 1.0, 10));
  ASSERT_TRUE(matching_scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 10));
  ASSERT_TRUE(non_matching_scope->SetNumericModifier("numeric_var", "seed", 5.0, 0.0, 10));

  auto definition = std::make_shared<StdEffectDefinition>(
      MakeEffectWithSelector("effect.id", ScopeType::SCOPE_TYPE_REGION,
                             "return VAR(numeric_var) >= 0 and VAR(core.class) == 'forest'",
                             "target:set_numeric_modifier('numeric_var', 4.0, 0.0)", "forest"));
  session.GetEffects().push_back(std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  StdEffectExecutor executor;
  executor.Execute(session, 10);

  auto matching_value = matching_scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(matching_value.has_value());
  EXPECT_EQ(*matching_value, 7.0);

  auto non_matching_value = non_matching_scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(non_matching_value.has_value());
  EXPECT_EQ(*non_matching_value, 5.0);
}

TEST(StdEffectExecutor, AppliesEffectOnlyToScopeMatchingSelectorScopeId) {
  StdSession session;
  auto target_scope =
      hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION, "target.region");
  auto other_scope = hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION, "other.region");
  ASSERT_TRUE(session.AddScope(target_scope));
  ASSERT_TRUE(session.AddScope(other_scope));

  ASSERT_TRUE(target_scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 10));
  ASSERT_TRUE(other_scope->SetNumericModifier("numeric_var", "seed", 5.0, 0.0, 10));

  auto definition = std::make_shared<StdEffectDefinition>(MakeEffectWithSelector(
      "effect.id", ScopeType::SCOPE_TYPE_REGION, "return VAR(numeric_var) >= 0",
      "target:set_numeric_modifier('numeric_var', 4.0, 0.0)", {}, "target.region"));
  session.GetEffects().push_back(std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  StdEffectExecutor executor;
  executor.Execute(session, 10);

  auto target_value = target_scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(target_value.has_value());
  EXPECT_EQ(*target_value, 7.0);

  auto other_value = other_scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(other_value.has_value());
  EXPECT_EQ(*other_value, 5.0);
}

}  // namespace hs::session
