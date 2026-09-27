#include <gtest/gtest.h>

#include <core/geometry/box.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/scope/scope_ut.hpp>
#include <core/terra/world.hpp>
#include <fstream>

#include "session.hpp"

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
                    std::filesystem::path("hic_sunt_session_create_test");
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root / "variables");
  {
    std::ofstream out(root / "variables" / "core.txt");
    out << "variables { id: \"core.turn\" numeric {} }\n";
    out << "variables { id: \"core.class\" string {} }\n";
    out << "variables { id: \"city.tag\" string {} }\n";
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

TEST(StdSessionCreate, CreateCityScope_IsNotRegisteredUntilAddScope) {
  auto session = MakePreparedSession();

  auto result = session.CreateCityScope("civ.id");
  ASSERT_TRUE(result.has_value());
  auto city = *result;

  // city should NOT be registered in session yet
  EXPECT_EQ(session.GetScopesById().find(city->GetId()), session.GetScopesById().end());

  // civ should NOT have this city as child yet
  auto civ = session.GetWorld()->GetCivilization("civ.id");
  ASSERT_NE(civ, nullptr);
  EXPECT_FALSE(civ->HasChildScope(ScopeType::SCOPE_TYPE_CITY, city->GetId()));

  // After AddScope, both session and civ should contain the city
  ASSERT_TRUE(session.AddScope(city));
  EXPECT_NE(session.GetScopesById().find(city->GetId()), session.GetScopesById().end());
  EXPECT_TRUE(civ->HasChildScope(ScopeType::SCOPE_TYPE_CITY, city->GetId()));

  auto stored = civ->GetChildScope(ScopeType::SCOPE_TYPE_CITY, city->GetId());
  EXPECT_EQ(stored, city);
}

TEST(StdSessionCreate, CreateImprovementScope_IsNotRegisteredUntilAddScope) {
  auto session = MakePreparedSession();

  // create city and register it so CreateImprovementScope can find it
  auto city_result = session.CreateCityScope("civ.id");
  ASSERT_TRUE(city_result.has_value());
  auto city = *city_result;
  ASSERT_TRUE(session.AddScope(city));

  // create improvement scope (it should be almost-orphaned and not registered)
  auto imp_result = session.CreateImprovementScope(city->GetId(), "farm");
  ASSERT_TRUE(imp_result.has_value());
  auto improvement = *imp_result;

  // improvement should NOT be registered in session yet
  EXPECT_EQ(session.GetScopesById().find(improvement->GetId()), session.GetScopesById().end());

  // civ should NOT have this improvement as child
  auto civ = session.GetWorld()->GetCivilization("civ.id");
  ASSERT_NE(civ, nullptr);
  EXPECT_FALSE(civ->HasChildScope(ScopeType::SCOPE_TYPE_IMPROVEMENT, improvement->GetId()));

  // After AddScope, improvement should be registered in session. It is not expected
  // to be added to civ children (only cities are promoted into civ in AddScope).
  ASSERT_TRUE(session.AddScope(improvement));
  EXPECT_NE(session.GetScopesById().find(improvement->GetId()), session.GetScopesById().end());
  EXPECT_FALSE(civ->HasChildScope(ScopeType::SCOPE_TYPE_IMPROVEMENT, improvement->GetId()));
}

}  // namespace hs::session
