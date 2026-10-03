#include <gtest/gtest.h>
#include <session/session.pb.h>

#include <core/geometry/box.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/terra/world.hpp>
#include <core/utils/serialize.hpp>
#include <utils/test_data.hpp>

#include "session.hpp"

namespace hs::session {

using StdSession = Session<StdBaseTypes>;
using StdWorld = terra::World<StdBaseTypes>;
using StdWorldPtr = std::shared_ptr<StdWorld>;
using StdRuleSet = ruleset::RuleSet<StdBaseTypes>;
using ScopeType = types::ScopeType;
using ::hs::test::GetTestDataFolder;

namespace {

struct PreparedSession {
  StdSession session;
  std::shared_ptr<StdRuleSet> ruleset;
};

StdWorldPtr MakeWorld() {
  using QRSBox = StdWorld::QRSBox;
  using QRSCoords = StdWorld::QRSCoords;
  using namespace geometry::literals;

  auto world = std::make_shared<StdWorld>();
  world->AddPlane("plane.id", QRSBox(QRSCoords{0_q, 0_r}, QRSCoords{0_q, 0_r}), 1, 1);
  world->GetOrCreateCivilization("civ.one");
  world->GetOrCreateCivilization("civ.two");
  return world;
}

PreparedSession MakePreparedSession(const std::filesystem::path& root) {
  auto ruleset = std::make_shared<StdRuleSet>();
  utils::ErrorsCollection errors;
  EXPECT_TRUE(ruleset->Load({root}, errors));

  StdSession session;
  EXPECT_TRUE(session.SetWorld(MakeWorld()));
  EXPECT_TRUE(session.SetRuleSet(ruleset));
  return PreparedSession{.session = std::move(session), .ruleset = std::move(ruleset)};
}

}  // namespace

TEST(StdSessionCreateJobScope, ValidatesArguments) {
  StdSession without_world;
  auto result = without_world.CreateJobScope("civ.one", "job.miller");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ERR_WORLD_MUST_BE_SET_FIRST);

  StdSession without_ruleset;
  ASSERT_TRUE(without_ruleset.SetWorld(MakeWorld()));
  result = without_ruleset.CreateJobScope("civ.one", "job.miller");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ERR_RULESET_MUST_BE_SET_FIRST);

  auto prepared = MakePreparedSession(GetTestDataFolder());
  result = prepared.session.CreateJobScope("", "job.miller");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ERR_NULL_ID);

  result = prepared.session.CreateJobScope("civ.one", "");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ERR_NULL_ID);

  result = prepared.session.CreateJobScope("civ.missing", "job.miller");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ERR_NO_SUCH_CIV);
}

TEST(StdSessionCreateJobScope, CreatesOrphanInstanceAndLazyClassAndGroups) {
  auto prepared = MakePreparedSession(GetTestDataFolder());
  auto& session = prepared.session;

  auto result = session.CreateJobScope("civ.one", "job.miller");
  ASSERT_TRUE(result.has_value());
  const auto job = *result;

  EXPECT_EQ(job->GetType(), ScopeType::SCOPE_TYPE_JOB);
  EXPECT_EQ(job->GetParent(), nullptr);
  EXPECT_FALSE(session.GetScopesById().contains(job->GetId()));

  const auto class_id = StdRuleSet::JobClassScopeId("civ.one", "job.miller");
  const auto child_group_id = StdRuleSet::GroupScopeId("civ.one", "group.child");
  const auto base_group_id = StdRuleSet::GroupScopeId("civ.one", "group.base");
  ASSERT_TRUE(session.GetScopesById().contains(class_id));
  ASSERT_TRUE(session.GetScopesById().contains(child_group_id));
  ASSERT_TRUE(session.GetScopesById().contains(base_group_id));

  const auto class_scope = session.GetScopesById().at(class_id);
  const auto child_group = session.GetScopesById().at(child_group_id);
  const auto base_group = session.GetScopesById().at(base_group_id);
  EXPECT_EQ(class_scope->GetParent()->GetId(), "civ.one");
  EXPECT_EQ(child_group->GetParent()->GetId(), "civ.one");
  EXPECT_EQ(base_group->GetParent()->GetId(), "civ.one");
  ASSERT_EQ(job->GetTagScopes().size(), 1u);
  EXPECT_EQ(job->GetTagScopes()[0], class_scope);
  ASSERT_EQ(class_scope->GetTagScopes().size(), 1u);
  EXPECT_EQ(class_scope->GetTagScopes()[0], child_group);
  ASSERT_EQ(child_group->GetTagScopes().size(), 1u);
  EXPECT_EQ(child_group->GetTagScopes()[0], base_group);

  ASSERT_TRUE(base_group->SetNumericModifier("group.value", "test", 7, 0));
  job->SetVariableDefinitions(prepared.ruleset->GetVariableDefinitions());
  const auto inherited_value = job->GetNumericValue("group.value");
  ASSERT_TRUE(inherited_value.has_value());
  EXPECT_EQ(*inherited_value, 7);
}

TEST(StdSessionCreateJobScope, ReusesScopesWithinCivilizationAndSeparatesCivilizations) {
  auto prepared = MakePreparedSession(GetTestDataFolder());
  auto& session = prepared.session;

  ASSERT_TRUE(session.CreateJobScope("civ.one", "job.miller").has_value());
  ASSERT_TRUE(session.CreateJobScope("civ.one", "job.miller").has_value());

  auto class_scopes = session.GetScopesByType().find(ScopeType::SCOPE_TYPE_JOB_CLASS);
  ASSERT_NE(class_scopes, session.GetScopesByType().end());
  EXPECT_EQ(class_scopes->second.size(), 1u);
  auto group_scopes = session.GetScopesByType().find(ScopeType::SCOPE_TYPE_JOB_GROUP);
  ASSERT_NE(group_scopes, session.GetScopesByType().end());
  EXPECT_EQ(group_scopes->second.size(), 2u);

  ASSERT_TRUE(session.CreateJobScope("civ.two", "job.miller").has_value());
  EXPECT_EQ(session.GetScopesByType().at(ScopeType::SCOPE_TYPE_JOB_CLASS).size(), 2u);
  EXPECT_EQ(session.GetScopesByType().at(ScopeType::SCOPE_TYPE_JOB_GROUP).size(), 4u);
  EXPECT_TRUE(session.GetScopesById().contains(StdRuleSet::GroupScopeId("civ.one", "group.base")));
  EXPECT_TRUE(session.GetScopesById().contains(StdRuleSet::GroupScopeId("civ.two", "group.base")));
}

TEST(StdSessionCreateJobScope, ClassAndGroupScopesSurviveSessionRoundTrip) {
  auto prepared = MakePreparedSession(GetTestDataFolder());
  ASSERT_TRUE(prepared.session.CreateJobScope("civ.one", "job.miller").has_value());

  proto::session::Session proto_session;
  SerializeTo(prepared.session, proto_session);
  auto parsed = ParseFrom(proto_session, serialize::To<StdSession>{});

  const auto class_id = StdRuleSet::JobClassScopeId("civ.one", "job.miller");
  const auto child_group_id = StdRuleSet::GroupScopeId("civ.one", "group.child");
  const auto base_group_id = StdRuleSet::GroupScopeId("civ.one", "group.base");
  ASSERT_TRUE(parsed.GetScopesById().contains(class_id));
  ASSERT_TRUE(parsed.GetScopesById().contains(child_group_id));
  ASSERT_TRUE(parsed.GetScopesById().contains(base_group_id));

  const auto class_scope = parsed.GetScopesById().at(class_id);
  const auto child_group = parsed.GetScopesById().at(child_group_id);
  ASSERT_EQ(class_scope->GetTagScopes().size(), 1u);
  EXPECT_EQ(class_scope->GetTagScopes()[0]->GetId(), child_group_id);
  ASSERT_EQ(child_group->GetTagScopes().size(), 1u);
  EXPECT_EQ(child_group->GetTagScopes()[0]->GetId(), base_group_id);
  EXPECT_EQ(class_scope->GetParent()->GetId(), "civ.one");
}

}  // namespace hs::session
