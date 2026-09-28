#include <gtest/gtest.h>

#include <core/utils/error_message.hpp>
#include <cstdlib>
#include <fstream>

#include "ruleset.hpp"

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
  WriteTextFile(root / "biomes" / "base.txt", "biomes { id: \"biome.one\" }\n");
  WriteTextFile(root / "biomes" / "nested" / "more.txt",
                "biome_features { id: \"feature.one\" }\n");
  WriteTextFile(root / "resources" / "a.txt", "resources { id: \"resource.one\" }\n");
  WriteTextFile(root / "effects" / "e.txt", "effects { id: \"effect.one\" }\n");

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

TEST(StdRuleSet, LaterFilesOverrideObjectsWithSameId) {
  const auto first = MakeTempDir("override_first");
  const auto second = MakeTempDir("override_second");
  WriteTextFile(first / "projects" / "a.txt",
                "projects { id: \"project.one\" script: \"res://first.gd\" }\n");
  WriteTextFile(second / "projects" / "b.txt",
                "projects { id: \"project.one\" script: \"res://second.gd\" }\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({first, second}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).id(), "project.one");
  EXPECT_EQ(ruleset.GetProjects().projects(0).script(), "res://second.gd");
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
  WriteTextFile(root / "variables" / "good.txt", "variables { id: \"var.one\" string {} }\n");
  WriteTextFile(root / "variables" / "bad.txt", "this is not protobuf text\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  EXPECT_TRUE(ruleset.GetVariableDefinitions()->IsStringVariable("var.one"));
  EXPECT_FALSE(ruleset.GetVariableDefinitions()->IsNumericVariable("var.one"));
}

TEST(StdRuleSet, LoadYamlImprovementsWithMapFieldsAsDictionaries) {
  const auto root = MakeTempDir("yaml_improvements_maps");
  WriteTextFile(root / "improvements" / "region_improvements.yaml",
                "improvements:\n"
                "  - id: core.improv.logging_1\n"
                "    jobs:\n"
                "      core.job.woodcutter: 1\n"
                "  - id: core.bld.palace\n"
                "    cost:\n"
                "      amounts:\n"
                "        core.res.stone: 10\n"
                "        core.res.wood: 10\n"
                "        core.res.workforce: 2\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetRegionImprovements().improvements_size(), 2);
  const auto& logging = ruleset.GetRegionImprovements().improvements(0);
  EXPECT_EQ(logging.id(), "core.improv.logging_1");
  ASSERT_TRUE(logging.jobs().contains("core.job.woodcutter"));
  EXPECT_EQ(logging.jobs().at("core.job.woodcutter"), 1);

  const auto& palace = ruleset.GetRegionImprovements().improvements(1);
  EXPECT_EQ(palace.id(), "core.bld.palace");
  ASSERT_TRUE(palace.cost().amounts().contains("core.res.stone"));
  ASSERT_TRUE(palace.cost().amounts().contains("core.res.wood"));
  ASSERT_TRUE(palace.cost().amounts().contains("core.res.workforce"));
  EXPECT_EQ(palace.cost().amounts().at("core.res.stone"), 10);
  EXPECT_EQ(palace.cost().amounts().at("core.res.wood"), 10);
  EXPECT_EQ(palace.cost().amounts().at("core.res.workforce"), 2);
}

TEST(StdRuleSet, LoadYamlJobsWithInputOutputMapsAsDictionaries) {
  const auto root = MakeTempDir("yaml_jobs_maps");
  WriteTextFile(root / "jobs" / "jobs.yaml",
                "jobs:\n"
                "  - id: core.job.woodcutter\n"
                "    input:\n"
                "      core.res.food: 1\n"
                "    output:\n"
                "      core.res.wood: 3\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetJobs().jobs_size(), 1);
  const auto& job = ruleset.GetJobs().jobs(0);
  EXPECT_EQ(job.id(), "core.job.woodcutter");
  ASSERT_TRUE(job.input().contains("core.res.food"));
  ASSERT_TRUE(job.output().contains("core.res.wood"));
  EXPECT_EQ(job.input().at("core.res.food"), 1);
  EXPECT_EQ(job.output().at("core.res.wood"), 3);
}

TEST(StdRuleSet, LoadMixedFormatsInLexicographicOrderByPathWithoutExtension) {
  const auto root = MakeTempDir("mixed_format_order");
  WriteTextFile(root / "projects" / "01_base.yaml",
                "projects:\n"
                "  - id: project.one\n"
                "    script: res://first.gd\n");
  WriteTextFile(root / "projects" / "02_override.txt",
                "projects { id: \"project.one\" script: \"res://second.gd\" }\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).script(), "res://second.gd");
}

TEST(StdRuleSet, LoadIgnoresFilesWithSameNameAndDifferentExtensionsInOneDirectory) {
  const auto root = MakeTempDir("same_name_different_extensions");
  WriteTextFile(root / "projects" / "01_valid.txt",
                "projects { id: \"project.valid\" script: \"res://valid.gd\" }\n");
  WriteTextFile(root / "projects" / "02_conflict.txt",
                "projects { id: \"project.conflict.txt\" script: \"res://txt.gd\" }\n");
  WriteTextFile(root / "projects" / "02_conflict.yaml",
                "projects:\n"
                "  - id: project.conflict.yaml\n"
                "    script: res://yaml.gd\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).id(), "project.valid");
}

TEST(StdRuleSet, LoadYamlScalarsUseYamlTypesBeforeProtobufParsing) {
  const auto root = MakeTempDir("yaml_scalar_types");
  WriteTextFile(root / "resources" / "01_quoted_string.yaml",
                "resources:\n"
                "  - id: \"true\"\n");
  WriteTextFile(root / "resources" / "02_unquoted_bool.yaml",
                "resources:\n"
                "  - id: true\n");
  WriteTextFile(root / "variables" / "01_bool.yaml",
                "variables:\n"
                "  - id: variable.bool\n"
                "    boolean: {}\n"
                "    immutable: true\n");
  WriteTextFile(root / "variables" / "02_string_bool.yaml",
                "variables:\n"
                "  - id: variable.string_bool\n"
                "    boolean: {}\n"
                "    immutable: \"true\"\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetResources().resources_size(), 1);
  EXPECT_EQ(ruleset.GetResources().resources(0).id(), "true");

  EXPECT_TRUE(ruleset.GetVariableDefinitions()->IsNumericVariable("variable.bool"));
  // Protobuf C++ JSON parsing enables legacy syntax internally, so quoted bools are
  // accepted for bool fields even though canonical ProtoJSON would use an unquoted bool.
  EXPECT_TRUE(ruleset.GetVariableDefinitions()->IsNumericVariable("variable.string_bool"));
}

TEST(StdRuleSet, LoadYamlRejectsAnchorsAndAliases) {
  const auto root = MakeTempDir("yaml_rejects_anchors");
  WriteTextFile(root / "resources" / "good.yaml",
                "resources:\n"
                "  - id: resource.good\n");
  WriteTextFile(root / "resources" / "bad.yaml",
                "resources:\n"
                "  - &resource_anchor\n"
                "    id: resource.bad\n"
                "  - *resource_anchor\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetResources().resources_size(), 1);
  EXPECT_EQ(ruleset.GetResources().resources(0).id(), "resource.good");
}

TEST(StdRuleSet, LoadJobsGeneratesNumericVariableDefinitions) {
  const auto root = MakeTempDir("job_variable_definitions");
  WriteTextFile(root / "resources" / "resources.txt",
                "resources { id: \"resource.wood\" }\n"
                "resources { id: \"resource.tools\" }\n"
                "resources { id: \"resource.food\" }\n");
  WriteTextFile(root / "jobs" / "jobs.txt",
                "jobs {\n"
                "  id: \"job.one\"\n"
                "  input { key: \"resource.wood\" value: 2 }\n"
                "  output { key: \"resource.tools\" value: 1 }\n"
                "}\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  const auto& definitions = ruleset.GetVariableDefinitions();
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/count"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/produces/resource.wood"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/produces/resource.tools"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/produces/resource.food"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/consumes/resource.wood"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/consumes/resource.tools"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/consumes/resource.food"));

  const auto count_definition = definitions->FindNumericVariable("job/job.one/count");
  ASSERT_TRUE(count_definition.has_value());
  EXPECT_EQ(count_definition->minimum, 0);
  EXPECT_EQ(count_definition->maximum, std::numeric_limits<StdBaseTypes::NumericValue>::max());
  EXPECT_TRUE(count_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(count_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_FALSE(count_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);

  const auto produces_definition =
      definitions->FindNumericVariable("job/job.one/produces/resource.food");
  ASSERT_TRUE(produces_definition.has_value());
  EXPECT_EQ(produces_definition->minimum, 0);
  EXPECT_TRUE(produces_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(produces_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_FALSE(produces_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);

  const auto consumes_definition =
      definitions->FindNumericVariable("job/job.one/consumes/resource.food");
  ASSERT_TRUE(consumes_definition.has_value());
  EXPECT_EQ(consumes_definition->minimum, 0);
  EXPECT_TRUE(consumes_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(consumes_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_FALSE(consumes_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);
}

TEST(StdRuleSet, LoadEffectsCreatesInlineImprovementEffects) {
  const auto root = MakeTempDir("inline_improvement_effects");
  WriteTextFile(root / "improvements" / "improvements.txt",
                "improvements {\n"
                "  id: \"mill\"\n"
                "  class_effect { lua: \"return VAR(mill.class.dep)\" }\n"
                "  instance_effect { lua: \"return VAR(mill.instance.dep)\" }\n"
                "}\n");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  const auto& effects = ruleset.GetAllEffectDefinitions();
  ASSERT_EQ(effects.size(), 2u);

  const auto& class_effect = effects[0];
  EXPECT_EQ(class_effect->GetId(), "mill/class.effect");
  EXPECT_EQ(class_effect->GetScopeType(), types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS);
  EXPECT_EQ(class_effect->GetData().selector().class_(), "mill");
  EXPECT_FALSE(class_effect->GetData().has_possible());
  EXPECT_TRUE(class_effect->GePossibleCode().has_value() == false);
  EXPECT_EQ(class_effect->GetData().effect().lua(), "return VAR(mill.class.dep)");

  const auto& instance_effect = effects[1];
  EXPECT_EQ(instance_effect->GetId(), "mill/instance.effect");
  EXPECT_EQ(instance_effect->GetScopeType(), types::ScopeType::SCOPE_TYPE_IMPROVEMENT);
  EXPECT_EQ(instance_effect->GetData().selector().class_(), "mill");
  EXPECT_TRUE(instance_effect->GetData().has_possible());
  EXPECT_EQ(instance_effect->GetData().possible().lua(), "return true");
  EXPECT_TRUE(instance_effect->GePossibleCode().has_value());
  EXPECT_EQ(instance_effect->GetData().effect().lua(), "return VAR(mill.instance.dep)");
}

}  // namespace hs::ruleset
