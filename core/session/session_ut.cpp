#include "session_ut.hpp"

#include <gtest/gtest.h>

#include <core/geometry/box.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/terra/world.hpp>
#include <core/utils/error_message.hpp>
#include <filesystem>
#include <fstream>

namespace hs::session::test {

namespace {

using StdWorld = terra::World<StdBaseTypes>;
using StdWorldPtr = std::shared_ptr<StdWorld>;
using StdRuleSet = ruleset::RuleSet<StdBaseTypes>;

StdWorldPtr MakeWorld() {
  using QRSBox = StdWorld::QRSBox;
  using QRSCoords = StdWorld::QRSCoords;
  using namespace geometry::literals;

  auto world = std::make_shared<StdWorld>();
  world->AddPlane("plane.id", QRSBox(QRSCoords{0_q, 0_r}, QRSCoords{0_q, 0_r}), 1, 1);
  return world;
}

}  // namespace

StdSession MakePreparedSession() {
  StdSession session;
  auto world = MakeWorld();
  auto civ = world->GetOrCreateCivilization("civ.id");
  (void)civ;

  const auto root =
      std::filesystem::temp_directory_path() / std::filesystem::path("hic_sunt_session_ut_ruleset");
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

}  // namespace hs::session::test
