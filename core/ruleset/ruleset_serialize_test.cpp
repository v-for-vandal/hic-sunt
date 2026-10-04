#include <gtest/gtest.h>
#include <ruleset/ruleset.pb.h>

#include <core/utils/serialize.hpp>
#include <filesystem>
#include <fstream>
#include <utils/test_data.hpp>

#include "ruleset.hpp"

namespace hs::ruleset {

using StdRuleSet = RuleSet<>;
using ::hs::test::GetTestDataFolder;

TEST(StdRuleSetSerialize, RoundTripPreservesLoadedVariables) {
  const auto root = std::filesystem::temp_directory_path() /
                    std::filesystem::path("hic_sunt_ruleset_serialize_test");
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root / "variables");
  {
    std::ofstream out(root / "variables" / "core.txt");
    out << "variables { id: \"core.turn\" numeric {} }\n";
    out << "variables { id: \"core.class\" string {} }\n";
  }

  StdRuleSet source;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(source.Load({root}, errors));

  proto::ruleset::RuleSet proto_ruleset;
  SerializeTo(source, proto_ruleset);

  auto parsed = ParseFrom(proto_ruleset, serialize::To<StdRuleSet>{});

  EXPECT_TRUE(parsed.GetVariableDefinitions()->IsNumericVariable("core.turn"));
  EXPECT_TRUE(parsed.GetVariableDefinitions()->IsStringVariable("core.class"));
}

TEST(StdRuleSetSerialize, RoundTripRestoresGroupsAndGeneratedEffects) {
  const auto root = GetTestDataFolder();

  StdRuleSet source;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(source.Load({root}, errors));

  proto::ruleset::RuleSet proto_ruleset;
  SerializeTo(source, proto_ruleset);
  auto parsed = ParseFrom(proto_ruleset, serialize::To<StdRuleSet>{});

  const auto group = parsed.FindGroupById("group.worker");
  ASSERT_TRUE(group.has_value());
  EXPECT_EQ(group->scope_type, types::ScopeType::SCOPE_TYPE_JOB_GROUP);
  EXPECT_EQ(group->group->id(), "group.worker");
  const auto effect = std::ranges::find_if(parsed.GetAllEffectDefinitions(), [](const auto& value) {
    return value->GetId() == "group.worker/group.effect";
  });
  ASSERT_NE(effect, parsed.GetAllEffectDefinitions().end());
  EXPECT_EQ((*effect)->GetScopeType(), types::ScopeType::SCOPE_TYPE_JOB_GROUP);
}

}  // namespace hs::ruleset
