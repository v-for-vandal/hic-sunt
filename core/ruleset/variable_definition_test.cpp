#include <gtest/gtest.h>

#include <core/ruleset/variable_definition.hpp>
#include <core/types/scope_type.hpp>

namespace hs::ruleset {

using StdVariableDefinitions = VariableDefinitions<StdBaseTypes>;
using VariableType = StdVariableDefinitions::VariableType;
using ParsedVariableDefinition = StdVariableDefinitions::ParsedVariableDefinition;

TEST(StdVariableDefinitions, ReportsMissingVariableType) {
  StdVariableDefinitions definitions;

  EXPECT_FALSE(definitions.IsVariable("missing"));
  EXPECT_EQ(definitions.GetVariableType("missing"), VariableType::kMissing);
}

TEST(StdVariableDefinitions, ReportsExistingVariableType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddNumericDefinition("numeric_var", {}));
  ASSERT_TRUE(definitions.AddStringDefinition("string_var", {}));

  EXPECT_TRUE(definitions.IsVariable("numeric_var"));
  EXPECT_TRUE(definitions.IsVariable("string_var"));
  EXPECT_EQ(definitions.GetVariableType("numeric_var"), VariableType::kNumeric);
  EXPECT_EQ(definitions.GetVariableType("string_var"), VariableType::kString);
}

TEST(StdVariableDefinitions, RejectsRedefinitionWithDifferentType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddNumericDefinition("shared_var", {}));

  auto result = definitions.AddStringDefinition("shared_var", {});
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdVariableDefinitions, AllowsRedefinitionWithSameType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddNumericDefinition("numeric_var", {}));

  NumericVariableDefinition<StdBaseTypes> replacement;
  replacement.minimum = 10;
  replacement.maximum = 20;

  auto result = definitions.AddNumericDefinition("numeric_var", replacement);
  ASSERT_TRUE(result.has_value());

  auto found = definitions.FindNumericVariable("numeric_var");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->minimum, 10);
  EXPECT_EQ(found->maximum, 20);
}

TEST(StdVariableDefinitions, MissingLookupsReturnUnexpected) {
  StdVariableDefinitions definitions;

  auto numeric = definitions.FindNumericVariable("missing_numeric");
  ASSERT_FALSE(numeric.has_value());
  EXPECT_EQ(numeric.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);

  auto string_ = definitions.FindStringVariable("missing_string");
  ASSERT_FALSE(string_.has_value());
  EXPECT_EQ(string_.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);
}

TEST(StdVariableDefinitions, LookupsReturnIncorrectTypeForExistingOppositeType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddNumericDefinition("numeric_var", {}));
  ASSERT_TRUE(definitions.AddStringDefinition("string_var", {}));

  const auto numeric_as_string = definitions.FindStringVariable("numeric_var");
  ASSERT_FALSE(numeric_as_string.has_value());
  EXPECT_EQ(numeric_as_string.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);

  const auto string_as_numeric = definitions.FindNumericVariable("string_var");
  ASSERT_FALSE(string_as_numeric.has_value());
  EXPECT_EQ(string_as_numeric.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);

  const auto found_string = definitions.FindVariable("string_var");
  ASSERT_TRUE(found_string.has_value());
  EXPECT_EQ(found_string->id, "string_var");
}

TEST(StdVariableDefinitions, ParameterizedLookupsReturnIncorrectTypeForExistingOppositeType) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.AddParameterizedNumericDefinition("numeric/{}", {}).has_value());
  ASSERT_TRUE(definitions.AddParameterizedStringDefinition("string/{}", {}).has_value());

  const auto numeric_as_string = definitions.FindStringVariable("numeric/@value");
  ASSERT_FALSE(numeric_as_string.has_value());
  EXPECT_EQ(numeric_as_string.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);

  const auto string_as_numeric = definitions.FindNumericVariable("string/@value");
  ASSERT_FALSE(string_as_numeric.has_value());
  EXPECT_EQ(string_as_numeric.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);

  const auto normalized_numeric_as_string = definitions.FindStringVariable("numeric/{}");
  ASSERT_FALSE(normalized_numeric_as_string.has_value());
  EXPECT_EQ(normalized_numeric_as_string.error(), ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);

  const auto missing = definitions.FindNumericVariable("missing/@value");
  ASSERT_FALSE(missing.has_value());
  EXPECT_EQ(missing.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);
}

TEST(StdVariableDefinitions, ParameterizedLookupWithInvalidFixedValueReturnsMissing) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.SetFixedParameterDomainValues(FixedParameterDomain::kJob, {"job.one"}));
  ASSERT_TRUE(definitions.AddParameterizedNumericDefinition("job/{job}/count", {}).has_value());

  const auto missing = definitions.FindNumericVariable("job/@job.two/count");
  ASSERT_FALSE(missing.has_value());
  EXPECT_EQ(missing.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);
}

TEST(StdVariableDefinitions, ParseStringVariableDefaultsAllowedScopesToAllScopeTypes) {
  proto::ruleset::Variable variable;
  variable.set_id("var.one");
  variable.mutable_string()->set_default_("var.default");

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto* definition = std::get_if<StringVariableDefinition<StdBaseTypes>>(&parsed);
  ASSERT_NE(definition, nullptr);

  EXPECT_EQ(definition->default_value, "var.default");
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_PLANE]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_REGION]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CELL]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CIV]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT_CLASS]);
}

TEST(StdVariableDefinitions, ParseNumericVariableLoadsAllowedScopesFromFilter) {
  proto::ruleset::Variable variable;
  variable.set_id("var.one");
  variable.mutable_numeric();
  variable.mutable_allowed_scopes()->add_scope_type_sets(
      proto::types::ScopeTypeSet::SCOPE_TYPE_SET_GEO);
  variable.mutable_allowed_scopes()->add_scope_types(proto::types::ScopeType::SCOPE_TYPE_CITY);

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto* definition = std::get_if<NumericVariableDefinition<StdBaseTypes>>(&parsed);
  ASSERT_NE(definition, nullptr);

  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_WORLD]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_PLANE]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_REGION]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CELL]);
  EXPECT_TRUE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CITY]);

  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_CIV]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_ARMY]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT]);
  EXPECT_FALSE(definition->allowed_scopes[types::ScopeType::SCOPE_TYPE_UNIT_CLASS]);
}

TEST(StdVariableDefinitions, ParseBooleanVariableAsNumericRange) {
  proto::ruleset::Variable variable;
  variable.set_id("var.bool");
  variable.mutable_boolean();

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto* definition = std::get_if<NumericVariableDefinition<StdBaseTypes>>(&parsed);
  ASSERT_NE(definition, nullptr);
  EXPECT_EQ(definition->minimum, 0);
  EXPECT_EQ(definition->maximum, 1);
}

TEST(StdVariableDefinitions, ParseVariableWithoutTypeReturnsError) {
  proto::ruleset::Variable variable;
  variable.set_id("var.invalid");

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto* error = std::get_if<ErrorCode>(&parsed);
  ASSERT_NE(error, nullptr);
  EXPECT_EQ(*error, ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
}

TEST(StdVariableDefinitions, ParseParameterizedVariableFromProto) {
  proto::ruleset::Variable variable;
  variable.set_id("job/{job}/consumes/{resource}");
  variable.mutable_numeric()->set_minimum(0);

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto* definition =
      std::get_if<ParameterizedNumericVariableDefinition<StdBaseTypes>>(&parsed);
  ASSERT_NE(definition, nullptr);

  EXPECT_EQ(definition->id, "job/{}/consumes/{}");
  EXPECT_EQ(definition->pattern, "job/{job}/consumes/{resource}");
  ASSERT_EQ(definition->parameters.size(), 2u);
  EXPECT_EQ(definition->parameters[0].kind, ParameterDomainKind::kFixed);
  EXPECT_EQ(definition->parameters[0].fixed_domain, FixedParameterDomain::kJob);
  EXPECT_EQ(definition->parameters[1].kind, ParameterDomainKind::kFixed);
  EXPECT_EQ(definition->parameters[1].fixed_domain, FixedParameterDomain::kResource);
}

TEST(StdVariableDefinitions, ParseParameterizedVariableRejectsUnknownFixedDomain) {
  proto::ruleset::Variable variable;
  variable.set_id("label/{thing}");
  variable.mutable_string();

  const ParsedVariableDefinition parsed = StdVariableDefinitions::ParseFromProto(variable);
  const auto* error = std::get_if<ErrorCode>(&parsed);
  ASSERT_NE(error, nullptr);
  EXPECT_EQ(*error, ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
}

TEST(StdVariableDefinitions, ParameterizedNumericVariableMatchesConcreteId) {
  StdVariableDefinitions definitions;
  NumericVariableDefinition<StdBaseTypes> definition;
  definition.minimum = 0;
  definition.maximum = 10;

  ASSERT_TRUE(definitions.SetFixedParameterDomainValues(FixedParameterDomain::kJob,
                                                        {"job.one", "job.two"}));
  ASSERT_TRUE(definitions.SetFixedParameterDomainValues(FixedParameterDomain::kResource,
                                                        {"resource.wood", "resource.food"}));

  const auto normalized_id =
      definitions.AddParameterizedNumericDefinition("job/{job}/produces/{resource}", definition);
  ASSERT_TRUE(normalized_id.has_value());
  EXPECT_EQ(*normalized_id, "job/{}/produces/{}");

  EXPECT_TRUE(definitions.IsNumericVariable("job/@job.one/produces/@resource.wood"));
  EXPECT_FALSE(definitions.IsNumericVariable("job/job.one/produces/resource.wood"));

  const auto found = definitions.FindNumericVariable("job/@job.one/produces/@resource.wood");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->id, "job/@job.one/produces/@resource.wood");
  EXPECT_EQ(found->minimum, 0);
  EXPECT_EQ(found->maximum, 10);

  const auto missing_value =
      definitions.FindNumericVariable("job/@job.one/produces/@resource.stone");
  ASSERT_FALSE(missing_value.has_value());
  EXPECT_EQ(missing_value.error(), ErrorCode::ERR_NO_SUCH_VARIABLE);
}

TEST(StdVariableDefinitions, OpenParameterizedNumericVariableMatchesUnknownConcreteId) {
  StdVariableDefinitions definitions;
  NumericVariableDefinition<StdBaseTypes> definition;
  definition.minimum = 0;

  const auto normalized_id =
      definitions.AddParameterizedNumericDefinition("job/{}/consumes/{}", definition);
  ASSERT_TRUE(normalized_id.has_value());
  EXPECT_EQ(*normalized_id, "job/{}/consumes/{}");

  const auto found = definitions.FindNumericVariable("job/@new.job/consumes/@new.resource");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->id, "job/@new.job/consumes/@new.resource");

  const auto invalid = definitions.FindNumericVariable("job/@bad/value/consumes/@new.resource");
  ASSERT_FALSE(invalid.has_value());
}

TEST(StdVariableDefinitions, ParameterizedNumericVariableQueriesByWildcard) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.SetFixedParameterDomainValues(FixedParameterDomain::kJob,
                                                        {"job.one", "job.two"}));
  ASSERT_TRUE(definitions.SetFixedParameterDomainValues(FixedParameterDomain::kResource,
                                                        {"resource.wood", "resource.food"}));
  ASSERT_TRUE(definitions.AddParameterizedNumericDefinition("job/{job}/consumes/{resource}", {})
                  .has_value());

  const auto resources = definitions.FindParameterizedNumericVariables("job/@job.one/consumes/@*");
  ASSERT_TRUE(resources.has_value());
  ASSERT_EQ(resources->size(), 2u);
  EXPECT_EQ((*resources)[0].normalized_id, "job/{}/consumes/{}");
  EXPECT_EQ((*resources)[0].variable_id, "job/@job.one/consumes/@resource.wood");
  ASSERT_EQ((*resources)[0].parameters.size(), 2u);
  EXPECT_EQ((*resources)[0].parameters[0].name, "job");
  EXPECT_EQ((*resources)[0].parameters[0].value, "job.one");
  EXPECT_EQ((*resources)[0].parameters[1].name, "resource");
  EXPECT_EQ((*resources)[0].parameters[1].value, "resource.wood");
  EXPECT_EQ((*resources)[1].variable_id, "job/@job.one/consumes/@resource.food");

  const auto jobs = definitions.FindParameterizedNumericVariables("job/@*/consumes/@resource.wood");
  ASSERT_TRUE(jobs.has_value());
  ASSERT_EQ(jobs->size(), 2u);
  EXPECT_EQ((*jobs)[0].variable_id, "job/@job.one/consumes/@resource.wood");
  EXPECT_EQ((*jobs)[1].variable_id, "job/@job.two/consumes/@resource.wood");
}

TEST(StdVariableDefinitions, ParameterizedVariableRejectsInvalidConcreteReferenceAndQuery) {
  StdVariableDefinitions definitions;
  ASSERT_TRUE(definitions.SetFixedParameterDomainValues(FixedParameterDomain::kJob, {"job.one"}));
  ASSERT_TRUE(definitions.SetFixedParameterDomainValues(FixedParameterDomain::kResource,
                                                        {"resource.wood"}));
  ASSERT_TRUE(definitions.AddParameterizedNumericDefinition("job/{job}/consumes/{resource}", {})
                  .has_value());

  const auto wildcard_in_concrete_id =
      definitions.FindNumericVariable("job/@*/consumes/@resource.wood");
  ASSERT_FALSE(wildcard_in_concrete_id.has_value());
  EXPECT_EQ(wildcard_in_concrete_id.error(), ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);

  const auto empty_parameter_value =
      definitions.FindParameterizedNumericVariables("job/@/consumes/@*");
  ASSERT_FALSE(empty_parameter_value.has_value());
  EXPECT_EQ(empty_parameter_value.error(), ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
}

TEST(StdVariableDefinitions, ParameterizedStringVariableMatchesConcreteIdAndQuery) {
  StdVariableDefinitions definitions;
  StringVariableDefinition<StdBaseTypes> definition;
  definition.default_value = "default.label";

  const auto normalized_id = definitions.AddParameterizedStringDefinition("label/{}", definition);
  ASSERT_TRUE(normalized_id.has_value());
  EXPECT_EQ(*normalized_id, "label/{}");

  const auto found = definitions.FindStringVariable("label/@thing.one");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->id, "label/@thing.one");
  EXPECT_EQ(found->default_value, "default.label");

  const auto instances = definitions.FindParameterizedStringVariables("label/@*");
  ASSERT_FALSE(instances.has_value());
  EXPECT_EQ(instances.error(), ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
}

TEST(StdVariableDefinitions, EmptyPlaceholderCreatesOpenParameterizedVariable) {
  StdVariableDefinitions definitions;

  const auto normalized_id = definitions.AddParameterizedNumericDefinition("relation/{}", {});
  ASSERT_TRUE(normalized_id.has_value());
  EXPECT_EQ(*normalized_id, "relation/{}");
  EXPECT_TRUE(definitions.IsNumericVariable("relation/@anything.valid"));
}

TEST(StdVariableDefinitions, ParameterizedVariableRejectsInvalidPatternAndValues) {
  StdVariableDefinitions definitions;

  const auto invalid_partial_segment =
      definitions.AddParameterizedNumericDefinition("job{job}/produces/{resource}", {});
  ASSERT_FALSE(invalid_partial_segment.has_value());
  EXPECT_EQ(invalid_partial_segment.error(), ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);

  const auto invalid_parameter_value =
      definitions.SetFixedParameterDomainValues(FixedParameterDomain::kJob, {"job/one"});
  ASSERT_FALSE(invalid_parameter_value.has_value());
  EXPECT_EQ(invalid_parameter_value.error(), ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);

  const auto invalid_static_segment =
      definitions.AddParameterizedNumericDefinition("job/{job}/@produces/{resource}", {});
  ASSERT_FALSE(invalid_static_segment.has_value());
  EXPECT_EQ(invalid_static_segment.error(), ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
}

}  // namespace hs::ruleset
