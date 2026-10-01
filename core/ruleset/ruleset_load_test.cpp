#include <gtest/gtest.h>

#include <core/utils/error_message.hpp>
#include <limits>
#include <utils/test_data.hpp>

#include "ruleset.hpp"

namespace hs::ruleset {

using StdRuleSet = RuleSet<StdBaseTypes>;
using ::hs::test::GetTestDataFolder;

TEST(StdRuleSet, LoadRecursivelyMergesFilesFromDirectories) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  EXPECT_EQ(ruleset.GetBiomes().biomes_size(), 1);
  EXPECT_EQ(ruleset.GetBiomes().biome_features_size(), 1);
  EXPECT_EQ(ruleset.GetResources().resources_size(), 1);
  EXPECT_EQ(ruleset.GetAllEffects().size(), 1u);
}

TEST(StdRuleSet, LoadRespectsOrderedDirectories) {
  const auto first = GetTestDataFolder("first");
  const auto second = GetTestDataFolder("second");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({first, second}, errors));

  ASSERT_EQ(ruleset.GetJobs().jobs_size(), 2);
  EXPECT_EQ(ruleset.GetJobs().jobs(0).id(), "job.first");
  EXPECT_EQ(ruleset.GetJobs().jobs(1).id(), "job.second");
}

TEST(StdRuleSet, LaterFilesOverrideObjectsWithSameId) {
  const auto first = GetTestDataFolder("first");
  const auto second = GetTestDataFolder("second");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({first, second}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).id(), "project.one");
  EXPECT_EQ(ruleset.GetProjects().projects(0).script(), "res://second.gd");
}

TEST(StdRuleSet, LoadIgnoresNonDirectoryPaths) {
  const auto root = GetTestDataFolder();
  const auto file_path = root / "not_a_directory.txt";

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({file_path, root}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).id(), "project.one");
}

TEST(StdRuleSet, LoadIgnoresUnreadableOrInvalidFiles) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  EXPECT_TRUE(ruleset.GetVariableDefinitions()->IsStringVariable("var.one"));
  EXPECT_FALSE(ruleset.GetVariableDefinitions()->IsNumericVariable("var.one"));
}

TEST(StdRuleSet, LoadYamlImprovementsWithMapFieldsAsDictionaries) {
  const auto root = GetTestDataFolder();

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
  const auto root = GetTestDataFolder();

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
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).script(), "res://second.gd");
}

TEST(StdRuleSet, LoadIgnoresFilesWithSameNameAndDifferentExtensionsInOneDirectory) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetProjects().projects_size(), 1);
  EXPECT_EQ(ruleset.GetProjects().projects(0).id(), "project.valid");
}

TEST(StdRuleSet, LoadYamlScalarsUseYamlTypesBeforeProtobufParsing) {
  const auto root = GetTestDataFolder();

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
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  ASSERT_EQ(ruleset.GetResources().resources_size(), 1);
  EXPECT_EQ(ruleset.GetResources().resources(0).id(), "resource.good");
}

TEST(StdRuleSet, LoadParameterizedJobVariablesFromVariableDefinitions) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  const auto& definitions = ruleset.GetVariableDefinitions();
  EXPECT_TRUE(definitions->IsNumericVariable("job/job.one/count"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/@job.one/produces/@resource.wood"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/@job.one/produces/@resource.tools"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/@job.one/produces/@resource.food"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/@job.one/consumes/@resource.wood"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/@job.one/consumes/@resource.tools"));
  EXPECT_TRUE(definitions->IsNumericVariable("job/@job.one/consumes/@resource.food"));
  EXPECT_FALSE(definitions->IsNumericVariable("job/job.one/produces/resource.wood"));

  const auto count_definition = definitions->FindNumericVariable("job/job.one/count");
  ASSERT_TRUE(count_definition.has_value());
  EXPECT_EQ(count_definition->minimum, 0);
  EXPECT_EQ(count_definition->maximum, std::numeric_limits<StdBaseTypes::NumericValue>::max());
  EXPECT_TRUE(count_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(count_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_FALSE(count_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);

  const auto produces_definition =
      definitions->FindNumericVariable("job/@job.one/produces/@resource.food");
  ASSERT_TRUE(produces_definition.has_value());
  EXPECT_EQ(produces_definition->minimum, 0);
  EXPECT_TRUE(produces_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(produces_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_FALSE(produces_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);

  const auto consumes_definition =
      definitions->FindNumericVariable("job/@job.one/consumes/@resource.food");
  ASSERT_TRUE(consumes_definition.has_value());
  EXPECT_EQ(consumes_definition->minimum, 0);
  EXPECT_TRUE(consumes_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(consumes_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_FALSE(consumes_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);

  const auto consumed_resources =
      definitions->FindParameterizedNumericVariables("job/@job.one/consumes/@*");
  ASSERT_TRUE(consumed_resources.has_value());
  ASSERT_EQ(consumed_resources->size(), 3u);
  EXPECT_EQ((*consumed_resources)[0].variable_id, "job/@job.one/consumes/@resource.wood");
  EXPECT_EQ((*consumed_resources)[1].variable_id, "job/@job.one/consumes/@resource.tools");
  EXPECT_EQ((*consumed_resources)[2].variable_id, "job/@job.one/consumes/@resource.food");
}

TEST(StdRuleSet, LoadEffectsCreatesInlineImprovementEffects) {
  const auto root = GetTestDataFolder();

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
