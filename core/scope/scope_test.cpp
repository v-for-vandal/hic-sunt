#include "scope.hpp"

#include <gtest/gtest.h>

#include <core/scope/scope_ut.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

namespace hs::scope {

using StdScope = Scope<StdBaseTypes>;
using StdScopePtr = ScopePtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using StdVariableDefinitionsPtr = hs::ruleset::VariableDefinitionsPtr<StdBaseTypes>;
using StdVariableDefinitionsConstPtr = hs::ruleset::VariableDefinitionsConstPtr<StdBaseTypes>;

TEST(StdScope, Create) {
  StdScope stack_scope;

  StdScopePtr ptr_scope;
}

TEST(StdScope, NumericVariable) {
  auto scope = test::MakeSimpleScope();

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "some_key", 1.0, 2.0, 0));
  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 3.0);  // add=1.0 * (1 + mult=2.0)
}

TEST(StdScope, ValueLookupsPreserveMissingAndIncorrectTypeErrors) {
  auto scope = test::MakeSimpleScope();

  const auto missing_numeric = scope->GetNumericValue("missing_var");
  ASSERT_FALSE(missing_numeric.has_value());
  EXPECT_EQ(missing_numeric.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);

  const auto string_as_numeric = scope->GetNumericValue("string_var");
  ASSERT_FALSE(string_as_numeric.has_value());
  EXPECT_EQ(string_as_numeric.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);

  const auto missing_string = scope->GetStringValue("missing_var");
  ASSERT_FALSE(missing_string.has_value());
  EXPECT_EQ(missing_string.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);

  const auto numeric_as_string = scope->GetStringValue("numeric_var");
  ASSERT_FALSE(numeric_as_string.has_value());
  EXPECT_EQ(numeric_as_string.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdScope, ScalarQueriesPreserveMissingAndIncorrectTypeErrors) {
  auto scope = test::MakeSimpleScope();

  const auto missing_numeric = scope->GetNumericQuery("missing_var");
  ASSERT_FALSE(missing_numeric.has_value());
  EXPECT_EQ(missing_numeric.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);

  const auto string_as_numeric = scope->GetNumericQuery("string_var");
  ASSERT_FALSE(string_as_numeric.has_value());
  EXPECT_EQ(string_as_numeric.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);

  const auto missing_string = scope->GetStringQuery("missing_var");
  ASSERT_FALSE(missing_string.has_value());
  EXPECT_EQ(missing_string.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);

  const auto numeric_as_string = scope->GetStringQuery("numeric_var");
  ASSERT_FALSE(numeric_as_string.has_value());
  EXPECT_EQ(numeric_as_string.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdScope, InheritanceParent) {
  StdScopePtr parent_scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_WORLD);
  StdScopePtr scope("test", types::ScopeType::SCOPE_TYPE_PLANE);

  ASSERT_TRUE(scope->SetParent(parent_scope));

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "some_key", 1.0, 1.0, 0));
  EXPECT_TRUE(parent_scope->SetNumericModifier("numeric_var", "other_key", 1.0, 1.0));
  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 6.0);  // add=(1+1) * (1 + mult=(1+1))
}

TEST(StdScope, InheritanceIncludesTagScopes) {
  StdScopePtr parent_scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_WORLD, "parent");
  StdScopePtr tag_scope("tag", types::ScopeType::SCOPE_TYPE_PLANE_CLASS);
  StdScopePtr scope("test", types::ScopeType::SCOPE_TYPE_PLANE);

  ASSERT_TRUE(scope->SetParent(parent_scope));
  ASSERT_TRUE(scope->AddTagLink("tag_name", tag_scope));

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "self", 1.0, 0.0));
  EXPECT_TRUE(parent_scope->SetNumericModifier("numeric_var", "parent", 2.0, 1.0));
  EXPECT_TRUE(tag_scope->SetNumericModifier("numeric_var", "tag", 3.0, 0.5));

  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 15.0);  // add=(1+2+3) * (1 + mult=(0+1+0.5))
}

TEST(StdScope, GraphTraversalSkipsAlreadyVisitedTagScopes) {
  StdScopePtr world_scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_WORLD, "world");
  StdScopePtr plane_class_scope =
      test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_PLANE_CLASS, "plane_class");
  StdScopePtr scope("test", types::ScopeType::SCOPE_TYPE_PLANE);

  ASSERT_TRUE(scope->SetParent(world_scope));
  ASSERT_TRUE(scope->AddTagLink("tag_name", plane_class_scope));
  ASSERT_TRUE(plane_class_scope->SetParent(world_scope));

  EXPECT_TRUE(world_scope->SetNumericModifier("numeric_var", "shared", 2.0, 1.0));

  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 4.0);  // shared scope contributes only once

  std::vector<test::NumericExplanation> explanations;
  scope->ExplainNumericVariable(
      "numeric_var", [&explanations](const auto& scope_id, const auto&, const auto& modifier,
                                     auto add, auto mult) {
        explanations.push_back(test::NumericExplanation{
            .scope_id = scope_id,
            .variable = "",
            .modifier = modifier,
            .add = add,
            .mult = mult,
        });
      });

  ASSERT_EQ(explanations.size(), 1u);
  EXPECT_EQ(explanations[0].scope_id, "world");
  EXPECT_EQ(explanations[0].modifier, "shared");

  auto modification_time = scope->GetModificationTime("numeric_var");
  ASSERT_TRUE(modification_time.has_value());
  EXPECT_EQ(*modification_time, 0u);
}

TEST(StdScope, AddTagLinkRejectsDuplicateScopeId) {
  StdScopePtr scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_PLANE, "scope");
  StdScopePtr tag_scope_a("shared", types::ScopeType::SCOPE_TYPE_PLANE_CLASS);
  StdScopePtr tag_scope_b("shared", types::ScopeType::SCOPE_TYPE_PLANE_CLASS);

  ASSERT_TRUE(scope->AddTagLink("tag_a", tag_scope_a));

  auto result = scope->AddTagLink("tag_b", tag_scope_b);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
}

TEST(StdScope, OpenParameterizedNumericQueryReturnsOnlyMaterializedValuesFromGraph) {
  auto mutable_definitions = std::make_shared<StdVariableDefinitions>();
  hs::ruleset::NumericVariableDefinition<StdBaseTypes> definition;
  ASSERT_TRUE(
      mutable_definitions->AddParameterizedNumericDefinition("job/{}/consumes/{}", definition)
          .has_value());
  StdVariableDefinitionsConstPtr definitions{
      std::static_pointer_cast<const StdVariableDefinitions>(mutable_definitions)};

  StdScopePtr parent("parent", types::ScopeType::SCOPE_TYPE_PLANE);
  parent->SetVariableDefinitions(definitions);
  StdScopePtr child("child", types::ScopeType::SCOPE_TYPE_REGION);
  ASSERT_TRUE(child->SetParent(parent));

  ASSERT_TRUE(parent->SetNumericModifier("job/{}/consumes/{}", "default", 5, 0, 1));

  const auto default_value = child->GetNumericValue("job/@job.unknown/consumes/@resource.wood");
  ASSERT_TRUE(default_value.has_value());
  EXPECT_EQ(*default_value, 5);

  auto empty_query = child->GetNumericQuery("job/@*/consumes/@resource.wood");
  ASSERT_TRUE(empty_query.has_value());
  EXPECT_TRUE(empty_query->empty());

  const auto wildcard_modifier =
      parent->SetNumericModifier("job/@*/consumes/@resource.wood", "base", 3, 0, 42);
  ASSERT_FALSE(wildcard_modifier.has_value());
  EXPECT_EQ(wildcard_modifier.error(), ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);

  ASSERT_TRUE(
      parent->SetNumericModifier("job/@job.farmer/consumes/@resource.wood", "base", 3, 0, 42));

  const auto values = child->GetNumericQuery("job/@*/consumes/@resource.wood");
  ASSERT_TRUE(values.has_value());
  ASSERT_EQ(values->size(), 1u);
  EXPECT_EQ((*values)[0].first, "job/@job.farmer/consumes/@resource.wood");
  EXPECT_EQ((*values)[0].second, 8);

  const auto modification_time = child->GetModificationTime("job/@*/consumes/@resource.wood");
  ASSERT_TRUE(modification_time.has_value());
  EXPECT_EQ(*modification_time, 42u);
}

TEST(StdScope, OpenParameterizedStringConcreteUsesDefaultValueAndNormalizedModifiers) {
  auto mutable_definitions = std::make_shared<StdVariableDefinitions>();
  hs::ruleset::StringVariableDefinition<StdBaseTypes> definition;
  definition.default_value = "label.default";
  ASSERT_TRUE(
      mutable_definitions->AddParameterizedStringDefinition("label/{}", definition).has_value());
  StdVariableDefinitionsConstPtr definitions{
      std::static_pointer_cast<const StdVariableDefinitions>(mutable_definitions)};

  StdScopePtr scope("scope", types::ScopeType::SCOPE_TYPE_WORLD);
  scope->SetVariableDefinitions(definitions);

  const auto value = scope->GetStringValue("label/@thing.any");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "label.default");

  ASSERT_TRUE(scope->SetStringModifier("label/@thing.any", "same_level", "label.same_level", 0));
  const auto same_level_value = scope->GetStringValue("label/@thing.any");
  ASSERT_TRUE(same_level_value.has_value());
  EXPECT_EQ(*same_level_value, "label.same_level");

  ASSERT_TRUE(scope->SetStringModifier("label/{}", "default", "label.normalized", 1));
  const auto normalized_value = scope->GetStringValue("label/@thing.any");
  ASSERT_TRUE(normalized_value.has_value());
  EXPECT_EQ(*normalized_value, "label.normalized");

  ASSERT_TRUE(scope->SetStringModifier("label/@thing.any", "override", "label.override", 2));
  const auto query = scope->GetStringQuery("label/@*");
  ASSERT_TRUE(query.has_value());
  ASSERT_EQ(query->size(), 1u);
  EXPECT_EQ((*query)[0].first, "label/@thing.any");
  EXPECT_EQ((*query)[0].second, "label.override");
}

TEST(StdScope, OpenParameterizedStringQueryMergesNormalizedAndConcreteModifiersByLevel) {
  auto mutable_definitions = std::make_shared<StdVariableDefinitions>();
  hs::ruleset::StringVariableDefinition<StdBaseTypes> definition;
  definition.default_value = "label.default";
  ASSERT_TRUE(
      mutable_definitions->AddParameterizedStringDefinition("label/{}", definition).has_value());
  StdVariableDefinitionsConstPtr definitions{
      std::static_pointer_cast<const StdVariableDefinitions>(mutable_definitions)};

  StdScopePtr scope("scope", types::ScopeType::SCOPE_TYPE_WORLD);
  scope->SetVariableDefinitions(definitions);

  ASSERT_TRUE(scope->SetStringModifier("label/{}", "normalized", "label.normalized", 10));
  ASSERT_TRUE(scope->SetStringModifier("label/@thing.any", "concrete", "label.concrete", 1));

  const auto scalar = scope->GetStringValue("label/@thing.any");
  ASSERT_TRUE(scalar.has_value());
  EXPECT_EQ(*scalar, "label.normalized");

  const auto query = scope->GetStringQuery("label/@*");
  ASSERT_TRUE(query.has_value());
  ASSERT_EQ(query->size(), 1u);
  EXPECT_EQ((*query)[0].first, "label/@thing.any");
  EXPECT_EQ((*query)[0].second, "label.normalized");
}

TEST(StdScope, ParameterizedNumericQueryReturnsMaterializedConcreteValues) {
  auto mutable_definitions = std::make_shared<StdVariableDefinitions>();
  hs::ruleset::NumericVariableDefinition<StdBaseTypes> definition;
  definition.minimum = 0;
  ASSERT_TRUE(mutable_definitions->SetFixedParameterDomainValues(
      hs::ruleset::FixedParameterDomain::kJob, {"job.one", "job.two"}));
  ASSERT_TRUE(mutable_definitions->SetFixedParameterDomainValues(
      hs::ruleset::FixedParameterDomain::kResource, {"resource.wood", "resource.food"}));
  ASSERT_TRUE(mutable_definitions
                  ->AddParameterizedNumericDefinition("job/{job}/produces/{resource}", definition)
                  .has_value());
  StdVariableDefinitionsConstPtr definitions{
      std::static_pointer_cast<const StdVariableDefinitions>(mutable_definitions)};

  StdScopePtr scope("scope", types::ScopeType::SCOPE_TYPE_WORLD);
  scope->SetVariableDefinitions(definitions);

  ASSERT_TRUE(scope->SetNumericModifier("job/@job.one/produces/@resource.wood", "base", 2, 0));

  const auto concrete_value = scope->GetNumericValue("job/@job.one/produces/@resource.wood");
  ASSERT_TRUE(concrete_value.has_value());
  EXPECT_EQ(*concrete_value, 2);

  const auto values = scope->GetNumericQuery("job/@job.one/produces/@*");
  ASSERT_TRUE(values.has_value());
  ASSERT_EQ(values->size(), 1u);
  EXPECT_EQ((*values)[0].first, "job/@job.one/produces/@resource.wood");
  EXPECT_EQ((*values)[0].second, 2);
}

}  // namespace hs::scope
