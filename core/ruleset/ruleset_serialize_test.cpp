#include <gtest/gtest.h>
#include <ruleset/ruleset.pb.h>

#include <core/utils/serialize.hpp>
#include <filesystem>
#include <fstream>

#include "ruleset.hpp"

namespace hs::ruleset {

using StdRuleSet = RuleSet<>;

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

}  // namespace hs::ruleset
