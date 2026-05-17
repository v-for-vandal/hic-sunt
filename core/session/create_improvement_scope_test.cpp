#include "session.hpp"

#include <gtest/gtest.h>

#include <core/geometry/box.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/scope/scope_ut.hpp>
#include <core/terra/world.hpp>
#include <fstream>

namespace hs::session {

using StdSession = Session<StdBaseTypes>;
using StdScope = scope::Scope<StdBaseTypes>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;
using StdWorld = terra::World<StdBaseTypes>;
using StdWorldPtr = std::shared_ptr<StdWorld>;
using StdRuleSet = ruleset::RuleSet<StdBaseTypes>;
using ScopeType = types::ScopeType;

namespace {

StdWorldPtr MakeWorld() {
  using QRSBox = StdWorld::QRSBox;
  using QRSCoords = StdWorld::QRSCoords;
  using namespace geometry::literals;

  auto world = std::make_shared<StdWorld>();
  world->AddPlane("plane.id", QRSBox(QRSCoords{0_q, 0_r}, QRSCoords{0_q, 0_r}), 1, 1);
  return world;
}

StdSession MakePreparedSession() {
  StdSession session;
  auto world = MakeWorld();
  auto civ = world->GetOrCreateCivilization("civ.id");
  (void)civ;

  const auto root = std::filesystem::temp_directory_path() /
                    std::filesystem::path("hic_sunt_create_improvement_scope_test");
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root / "variables");
  {
    std::ofstream out(root / "variables" / "core.txt");
    out << "variables { id: \"core.turn\" numeric {} }\n";
    out << "variables { id: \"core.class\" string {} }\n";
  }

  auto ruleset = std::make_shared<StdRuleSet>();
  utils::ErrorsCollection errors;
  EXPECT_TRUE(ruleset->Load({root}, errors));

  EXPECT_TRUE(ruleset->GetVariableDefinitions()->IsNumericVariable("core.turn"));
  EXPECT_TRUE(ruleset->GetVariableDefinitions()->IsStringVariable("core.class"));

  EXPECT_TRUE(session.SetWorld(world));
  EXPECT_TRUE(session.SetRuleSet(ruleset));

  return session;
}

}  // namespace

TEST(StdSessionCreateImprovementScope, RejectsNullCivilizationId) {
  auto session = MakePreparedSession();

  auto result = session.CreateImprovementScope("", "farm");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_NULL_ID);
}

TEST(StdSessionCreateImprovementScope, RejectsNullImprovementClass) {
  auto session = MakePreparedSession();

  auto result = session.CreateImprovementScope("civ.id", "");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_NULL_ID);
}

TEST(StdSessionCreateImprovementScope, RejectsUnknownCivilization) {
  auto session = MakePreparedSession();

  auto result = session.CreateImprovementScope("missing.civ", "farm");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_NO_SUCH_CIV);
}

TEST(StdSessionCreateImprovementScope, RegistersImprovementAndClassScope) {
  auto session = MakePreparedSession();

  auto result = session.CreateImprovementScope("civ.id", "farm");
  ASSERT_TRUE(result.has_value());

  const auto improvement_scope = *result;
  EXPECT_EQ(improvement_scope->GetType(), ScopeType::SCOPE_TYPE_IMPROVEMENT);
  EXPECT_TRUE(session.GetScopesById().contains(improvement_scope->GetId()));

  auto by_type_it = session.GetScopesByType().find(ScopeType::SCOPE_TYPE_IMPROVEMENT);
  ASSERT_NE(by_type_it, session.GetScopesByType().end());
  EXPECT_EQ(by_type_it->second.size(), 1u);

  const auto class_scope_id = std::string{"civ/civ.id/iclass/farm"};
  auto class_it = session.GetScopesById().find(class_scope_id);
  ASSERT_NE(class_it, session.GetScopesById().end());
  EXPECT_EQ(class_it->second->GetType(), ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS);
}

TEST(StdSessionCreateImprovementScope, ReusesExistingImprovementClassScope) {
  auto session = MakePreparedSession();

  auto first_result = session.CreateImprovementScope("civ.id", "farm");
  ASSERT_TRUE(first_result.has_value());

  auto second_result = session.CreateImprovementScope("civ.id", "farm");
  ASSERT_TRUE(second_result.has_value());

  auto by_type_it = session.GetScopesByType().find(ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS);
  ASSERT_NE(by_type_it, session.GetScopesByType().end());
  EXPECT_EQ(by_type_it->second.size(), 1u);
}

TEST(StdSessionCreateImprovementScope, SetsCoreClassModifierAndClassTag) {
  auto session = MakePreparedSession();

  auto result = session.CreateImprovementScope("civ.id", "farm");
  ASSERT_TRUE(result.has_value());

  const auto improvement_scope = *result;

  // Without parent, improvement_scope has no definitions. add it to some region
  using WorldType = std::decay_t<decltype(*(session.GetWorld()))>;
  auto region_ptr = session.GetWorld()->GetPlane("plane.id")->GetRegions().begin()->second;
  ASSERT_TRUE(improvement_scope->SetParent(region_ptr->GetSurface().GetCell(WorldType::QRSCoords::MakeCoords(0,0)).GetScope()));

  auto class_value = improvement_scope->GetStringValue("core.class");
  ASSERT_TRUE(class_value.has_value());
  EXPECT_EQ(*class_value, "farm");

  std::vector<scope::test::StringExplanation> explanations;
  improvement_scope->ExplainStringVariable(
      "core.class", [&explanations](const auto& scope_id, const auto& variable,
                                      const auto& modifier, const auto& value, auto level) {
        explanations.push_back(scope::test::StringExplanation{
            .scope_id = scope_id,
            .variable = variable,
            .modifier = modifier,
            .value = value,
            .level = level,
        });
      });

  ASSERT_FALSE(explanations.empty());
}

}  // namespace hs::session
