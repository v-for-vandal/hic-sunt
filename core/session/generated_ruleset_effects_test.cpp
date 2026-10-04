#include <gtest/gtest.h>

#include <core/geometry/box.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/terra/world.hpp>
#include <utils/test_data.hpp>

#include "session.hpp"

namespace hs::session {

namespace {

using StdSession = Session<StdBaseTypes>;
using StdWorld = terra::World<StdBaseTypes>;
using StdRuleSet = ruleset::RuleSet<StdBaseTypes>;
using ::hs::test::GetCommonTestDataFolder;
using ::hs::test::GetTestDataFolder;

std::shared_ptr<StdWorld> MakeWorld() {
  using QRSBox = StdWorld::QRSBox;
  using QRSCoords = StdWorld::QRSCoords;
  using namespace geometry::literals;

  auto world = std::make_shared<StdWorld>();
  world->AddPlane("plane.id", QRSBox(QRSCoords{0_q, 0_r}, QRSCoords{0_q, 0_r}), 1, 1);
  world->GetOrCreateCivilization("civ.id");
  return world;
}

}  // namespace

TEST(StdSessionGeneratedRulesetEffects, AppliesImprovementJobsAndJobResourceFlows) {
  const auto common_root = GetCommonTestDataFolder("rulesets/job-count");
  const auto root = GetTestDataFolder();

  auto ruleset = std::make_shared<StdRuleSet>();
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset->Load({common_root, root}, errors));

  StdSession session;
  ASSERT_TRUE(session.SetWorld(MakeWorld()));
  ASSERT_TRUE(session.SetRuleSet(ruleset));

  auto city_result = session.CreateCityScope("civ.id");
  ASSERT_TRUE(city_result.has_value());
  ASSERT_TRUE(session.AddScope(*city_result));
  ASSERT_TRUE(
      session.CreateImprovementScope((*city_result)->GetId(), "improvement.mill").has_value());
  ASSERT_TRUE(session.CreateJobScope("civ.id", "job.miller").has_value());

  ASSERT_TRUE(session.AdvanceNextTurn().has_value());

  const auto improvement_class_id =
      StdRuleSet::ImprovementClassScopeId("civ.id", "improvement.mill");
  const auto job_class_id = StdRuleSet::JobClassScopeId("civ.id", "job.miller");
  const auto improvement_class = session.GetScopesById().at(improvement_class_id);
  const auto job_class = session.GetScopesById().at(job_class_id);

  const auto job_count = improvement_class->GetNumericValue("job/@job.miller/count");
  ASSERT_TRUE(job_count.has_value());
  EXPECT_EQ(*job_count, 2);
  const auto consumed = job_class->GetNumericValue("consumes/@resource.grain");
  ASSERT_TRUE(consumed.has_value());
  EXPECT_EQ(*consumed, 3);
  const auto produced = job_class->GetNumericValue("produces/@resource.flour");
  ASSERT_TRUE(produced.has_value());
  EXPECT_EQ(*produced, 5);
}

}  // namespace hs::session
