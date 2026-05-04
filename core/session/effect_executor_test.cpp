#include "effect_executor.hpp"

#include <gtest/gtest.h>

#include <ruleset/effect.pb.h>

#include <memory>
#include <vector>

#include <core/ruleset/variable_definition.hpp>

#include <core/scope/scope_ut.hpp>
#include <core/ruleset/ruleset_ut.hpp>

namespace hs::session {

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
  auto scope = hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = hs::ruleset::test::MakeEffectDefinition(
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
  auto scope = hs::scope::test::MakeSeededScope(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = hs::ruleset::test::MakeEffectDefinition(
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
