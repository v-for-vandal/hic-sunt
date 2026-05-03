#include "ruleset.hpp"

#include <gtest/gtest.h>

#include <core/utils/error_message.hpp>
#include <cstdlib>
#include <fstream>

namespace hs::ruleset {

using StdRuleSet = RuleSet<StdBaseTypes>;

namespace {

std::filesystem::path MakeTempDir(const std::string& name) {
  const auto root = std::filesystem::temp_directory_path() /
                    std::filesystem::path("hic_sunt_ruleset_tests") / name;
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root);
  return root;
}

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream out(path);
  out << content;
}

}  // namespace

TEST(StdRuleSet, LoadRecursivelyMergesFilesFromDirectories) {
  const auto root = MakeTempDir("recursive_merge");
  WriteTextFile(root / "biomes" / "base.txt",
                "biomes { id: \"biome.one\" }\n");
  WriteTextFile(root / "biomes" / "nested" / "more.txt",
                "biome_features { id: \"feature.one\" }\n");
  WriteTextFile(root / "resources" / "a.txt",
                "resources { id: \"resource.one\" }\n");
  WriteTextFile(root / "effects" / "e.txt",
                "effects { id: \"effect.one\" }\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  EXPECT_EQ(ruleset.GetBiomes().biomes_size(), 1);
  EXPECT_EQ(ruleset.GetBiomes().biome_features_size(), 1);
  EXPECT_EQ(ruleset.GetResources().resources_size(), 1);
  EXPECT_EQ(ruleset.GetAllEffects().size(), 1u);
}

TEST(StdRuleSet, LoadRespectsOrderedDirectories) {
  const auto first = MakeTempDir("ordered_first");
  const auto second = MakeTempDir("ordered_second");
  WriteTextFile(first / "jobs" / "a.txt", "jobs { id: \"job.first\" }\n");
  WriteTextFile(second / "jobs" / "b.txt", "jobs { id: \"job.second\" }\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({first, second}, errors));

  ASSERT_EQ(ruleset.GetJobs().jobs_size(), 2);
  EXPECT_EQ(ruleset.GetJobs().jobs(0).id(), "job.first");
  EXPECT_EQ(ruleset.GetJobs().jobs(1).id(), "job.second");
}

TEST(StdRuleSet, LoadIgnoresNonDirectoryPaths) {
  const auto root = MakeTempDir("ignore_non_directory");
  const auto file_path = root / "not_a_directory.txt";
  WriteTextFile(file_path, "ignored");
  WriteTextFile(root / "projects" / "p.txt",
                "projects { id: \"project.one\" script: \"res://a.gd\" }\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({file_path, root}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).id(), "project.one");
}

TEST(StdRuleSet, LoadIgnoresUnreadableOrInvalidFiles) {
  const auto root = MakeTempDir("ignore_invalid_files");
  WriteTextFile(root / "variables" / "good.txt",
                "variables { id: \"var.one\" string {} }\n");
  WriteTextFile(root / "variables" / "bad.txt", "this is not protobuf text\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  EXPECT_TRUE(ruleset.GetVariableDefinitions().IsStringVariable("var.one"));
  EXPECT_FALSE(ruleset.GetVariableDefinitions().IsNumericVariable("var.one"));
}

}  // namespace hs::ruleset
