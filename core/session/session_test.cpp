#include "session.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <memory>

#include <ruleset/effect.pb.h>
#include <core/ruleset/variable_definition.hpp>

#include <core/geometry/box.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/terra/world.hpp>

namespace hs::session {

using StdSession = Session<StdBaseTypes>;
using StdScope = scope::Scope<StdBaseTypes>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;
using StdWorld = terra::World<StdBaseTypes>;
using StdWorldPtr = std::shared_ptr<StdWorld>;
using StdEffectDefinition = ruleset::EffectDefinition<StdBaseTypes>;
using StdEffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using ScopeType = types::ScopeType;

namespace {

StdScopePtr MakeScope(const std::string& id, ScopeType type) {
  return StdScopePtr(id, type);
}

StdScopePtr MakeEffectScope(const std::string& id, ScopeType type) {
  auto definitions = std::make_shared<StdVariableDefinitions>();
  definitions->AddNumericDefinition("numeric_var", {});

  StdScopePtr scope(id, type);
  scope->SetVariableDefinitions(definitions);
  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 2.0, 0.0));
  return scope;
}

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

StdWorldPtr MakeWorld() {
  using QRSBox = StdWorld::QRSBox;
  using QRSCoords = StdWorld::QRSCoords;
  using namespace geometry::literals;

  auto world = std::make_shared<StdWorld>();
  world->AddPlane(
      "plane.id", QRSBox(QRSCoords{0_q, 0_r}, QRSCoords{0_q, 0_r}), 1, 1);
  return world;
}

}  // namespace

TEST(StdSession, AddScopeIndexesByIdAndType) {
  StdSession session;
  auto scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_REGION);

  ASSERT_TRUE(session.AddScope(scope));

  auto by_id_it = session.GetScopesById().find("scope.id");
  ASSERT_NE(by_id_it, session.GetScopesById().end());
  EXPECT_EQ(by_id_it->second, scope);

  auto by_type_it = session.GetScopesByType().find(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_NE(by_type_it, session.GetScopesByType().end());
  ASSERT_EQ(by_type_it->second.size(), 1u);
  EXPECT_EQ(by_type_it->second[0], scope);
}

TEST(StdSession, AddScopeAllowsReaddingSamePointer) {
  StdSession session;
  auto scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_REGION);

  ASSERT_TRUE(session.AddScope(scope));
  ASSERT_TRUE(session.AddScope(scope));

  auto by_type_it = session.GetScopesByType().find(ScopeType::SCOPE_TYPE_REGION);
  ASSERT_NE(by_type_it, session.GetScopesByType().end());
  ASSERT_EQ(by_type_it->second.size(), 1u);
}

TEST(StdSession, AddScopeRejectsDifferentPointerWithSameId) {
  StdSession session;
  auto first_scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_REGION);
  auto second_scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_REGION);

  ASSERT_TRUE(session.AddScope(first_scope));

  auto result = session.AddScope(second_scope);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_SCOPE_ALREADY_EXISTS);

  auto by_id_it = session.GetScopesById().find("scope.id");
  ASSERT_NE(by_id_it, session.GetScopesById().end());
  EXPECT_EQ(by_id_it->second, first_scope);
}

TEST(StdSession, AddScopeRejectsUnspecifiedType) {
  StdSession session;
  auto scope = MakeScope("scope.id", ScopeType::SCOPE_TYPE_UNSPECIFIED);

  auto result = session.AddScope(scope);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_SCOPE_TYPE_UNSPECIFIED);

  EXPECT_TRUE(session.GetScopesById().empty());
  EXPECT_TRUE(session.GetScopesByType().empty());
}

TEST(StdSession, SetWorldRegistersAllScopesRecursively) {
  StdSession session;
  auto world = MakeWorld();

  auto result = session.SetWorld(world);
  ASSERT_TRUE(result.has_value());

  EXPECT_TRUE(session.GetScopesByType().contains(ScopeType::SCOPE_TYPE_WORLD));
  EXPECT_TRUE(session.GetScopesByType().contains(ScopeType::SCOPE_TYPE_PLANE));
  EXPECT_TRUE(session.GetScopesByType().contains(ScopeType::SCOPE_TYPE_REGION));
  EXPECT_TRUE(session.GetScopesByType().contains(ScopeType::SCOPE_TYPE_CELL));

  EXPECT_EQ(session.GetScopesByType().at(ScopeType::SCOPE_TYPE_WORLD).size(), 1u);
  EXPECT_EQ(session.GetScopesByType().at(ScopeType::SCOPE_TYPE_PLANE).size(), 1u);
  EXPECT_FALSE(session.GetScopesByType().at(ScopeType::SCOPE_TYPE_REGION).empty());
  EXPECT_FALSE(session.GetScopesByType().at(ScopeType::SCOPE_TYPE_CELL).empty());

  EXPECT_TRUE(session.GetScopesById().contains(world->GetScope()->GetId()));
}

TEST(StdSession, SetWorldCanOnlyBeCalledOnce) {
  StdSession session;
  auto first_world = MakeWorld();
  auto second_world = MakeWorld();

  ASSERT_TRUE(session.SetWorld(first_world).has_value());

  auto result = session.SetWorld(second_world);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_WORLD_ALREADY_SET);
}

TEST(StdSession, AdvanceNextTurnIncrementsTurnAndExecutesEffects) {
  using StdRuleSet = ruleset::RuleSet<StdBaseTypes>;

  StdSession session;
  auto world = MakeWorld();
  ASSERT_TRUE(session.SetWorld(world));
  ASSERT_TRUE(session.SetRuleSet(std::make_shared<StdRuleSet>()));

  auto scope = MakeEffectScope("scope.id", ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(session.AddScope(scope));

  auto definition = MakeEffectDefinition(
      "effect.id", ScopeType::SCOPE_TYPE_REGION,
      "return VAR(numeric_var) >= 0",
      "target:set_numeric_modifier('numeric_var', 4.0, 0.0)");
  session.GetEffects().push_back(
      std::make_shared<EffectInstance<StdBaseTypes>>(definition));

  ASSERT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 3.0, 0.0, 1));

  session.AdvanceNextTurn();

  EXPECT_EQ(session.GetCurrentTurn(), 1u);
  auto numeric_value = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(numeric_value.has_value());
  EXPECT_EQ(*numeric_value, 7.0);
}

TEST(StdSession, SetRuleSetBuildsEffectInstances) {
  using StdRuleSet = ruleset::RuleSet<StdBaseTypes>;

  const auto root = std::filesystem::temp_directory_path() /
                    std::filesystem::path("hic_sunt_session_ruleset_test");
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root / "effects");
  {
    std::ofstream out(root / "effects" / "effect.txt");
    out << "effects { id: \"effect.id\" possible { lua: \"return true\" } effect { lua: \"return\" } }\n";
  }

  auto ruleset = std::make_shared<StdRuleSet>();
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset->Load({root}, errors));

  StdSession session;
  auto result = session.SetRuleSet(ruleset);
  ASSERT_TRUE(result.has_value());
}

}  // namespace hs::session
