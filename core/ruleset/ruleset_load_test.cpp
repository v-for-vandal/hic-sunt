#include <gtest/gtest.h>

#include <core/utils/error_message.hpp>
#include <filesystem>
#include <fstream>
#include <limits>
#include <utils/test_data.hpp>

#include "ruleset.hpp"

namespace hs::ruleset {

using StdRuleSet = RuleSet<StdBaseTypes>;
using ::hs::test::GetTestDataFolder;

namespace {

bool HasErrorContaining(const utils::ErrorsCollection& errors, std::string_view text) {
  return std::ranges::any_of(errors.errors, [text](const auto& error) {
    return error.message.find(text) != std::string::npos;
  });
}

const StdRuleSet::GroupDefinition FindGroup(const StdRuleSet& ruleset, std::string_view id) {
  const auto group = ruleset.FindGroupById(std::string{id});
  EXPECT_TRUE(group.has_value());
  return group.value();
}

const EffectDefinition<StdBaseTypes>* FindEffect(const StdRuleSet& ruleset, std::string_view id) {
  const auto found =
      std::ranges::find_if(ruleset.GetAllEffectDefinitions(),
                           [id](const auto& effect) { return effect->GetId() == id; });
  if (found == ruleset.GetAllEffectDefinitions().end()) {
    ADD_FAILURE() << "Missing effect " << id;
    return nullptr;
  }
  return found->get();
}

}  // namespace

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
  EXPECT_TRUE(count_definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP]);
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

TEST(StdRuleSet, DumpVariablesCsvExpandsFixedParametersAndKeepsOpenParameters) {
  const auto root = GetTestDataFolder();
  const auto csv_path = std::filesystem::temp_directory_path() / "hic_sunt_variables.csv";

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));
  ASSERT_TRUE(ruleset.DumpVariablesCsv(csv_path, errors));

  std::ifstream input(csv_path);
  ASSERT_TRUE(input.is_open());
  std::string csv((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

  EXPECT_TRUE(csv.starts_with("name,type\n"));
  EXPECT_NE(csv.find("job/@job.one/produces/@resource.wood,numeric\n"), std::string::npos);
  EXPECT_NE(csv.find("relation/{}/score,numeric\n"), std::string::npos);
}

TEST(StdRuleSet, GroupDefinitionsPreserveOrderedRootOverrides) {
  const auto first = GetTestDataFolder("first");
  const auto second = GetTestDataFolder("second");

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({first, second}, errors));
  ASSERT_EQ(ruleset.GetRegionImprovements().improvement_groups_size(), 1);
  EXPECT_EQ(FindGroup(ruleset, "group.shared").group->name().key(), "second");
}

TEST(StdRuleSet, RejectsGroupIdUsedByBothGroupTypesAfterMerge) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  EXPECT_FALSE(ruleset.Load({root}, errors));
  EXPECT_TRUE(HasErrorContaining(errors, "Group id 'group.shared' is not unique"));
}

TEST(StdRuleSet, RejectsUnknownAndWrongTypeGroupReferences) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  EXPECT_FALSE(ruleset.Load({root}, errors));
  EXPECT_TRUE(HasErrorContaining(
      errors, "Improvement 'improvement.one' references group 'group.job' of type"));
  EXPECT_TRUE(HasErrorContaining(errors, "Job 'job.one' references unknown group 'group.missing'"));
}

TEST(StdRuleSet, RejectsDuplicateGroupReferences) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  EXPECT_FALSE(ruleset.Load({root}, errors));
  EXPECT_TRUE(HasErrorContaining(
      errors, "Improvement 'improvement.one' contains duplicate group reference 'group.child'"));
  EXPECT_TRUE(HasErrorContaining(
      errors, "improvement group 'group.child' contains duplicate group reference 'group.base'"));
}

TEST(StdRuleSet, RejectsCyclesInGroupGraphWithUsefulError) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  EXPECT_FALSE(ruleset.Load({root}, errors));
  EXPECT_TRUE(HasErrorContaining(
      errors, "Cycle detected in improvement group graph: group.a -> group.b -> group.a"));
}

TEST(StdRuleSet, CreatesJobImprovementAndGroupEffects) {
  const auto root = GetTestDataFolder();

  StdRuleSet ruleset;
  utils::ErrorsCollection errors;
  ASSERT_TRUE(ruleset.Load({root}, errors));

  const auto improvement_jobs = FindEffect(ruleset, "improvement.mill/jobs.effect");
  ASSERT_NE(improvement_jobs, nullptr);
  EXPECT_EQ(improvement_jobs->GetScopeType(), types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS);
  EXPECT_EQ(improvement_jobs->GetData().selector().class_(), "improvement.mill");
  EXPECT_NE(improvement_jobs->GetData().effect().lua().find("job/job.miller/count', 2"),
            std::string::npos);

  const auto job_resources = FindEffect(ruleset, "job.miller/resources.effect");
  ASSERT_NE(job_resources, nullptr);
  EXPECT_EQ(job_resources->GetScopeType(), types::ScopeType::SCOPE_TYPE_JOB_CLASS);
  EXPECT_NE(job_resources->GetData().effect().lua().find("consumes/@resource.grain', 3"),
            std::string::npos);
  EXPECT_NE(job_resources->GetData().effect().lua().find("produces/@resource.flour', 5"),
            std::string::npos);
  EXPECT_NE(job_resources->GetData().effect().lua().find("produces/@resource.o\\'neil', 1"),
            std::string::npos);
  EXPECT_FALSE(job_resources->IsBroken());
  ASSERT_EQ(job_resources->GetDependencies().size(), 1u);
  EXPECT_EQ(job_resources->GetDependencies()[0].raw_id, "core.class");

  EXPECT_EQ(FindEffect(ruleset, "group.production/group.effect")->GetScopeType(),
            types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP);
  EXPECT_EQ(FindEffect(ruleset, "group.workers/group.effect")->GetScopeType(),
            types::ScopeType::SCOPE_TYPE_JOB_GROUP);
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
