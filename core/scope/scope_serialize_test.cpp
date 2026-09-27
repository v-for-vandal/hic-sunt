#include <gtest/gtest.h>
#include <scope/scope.pb.h>

#include <core/ruleset/variable_definition.hpp>
#include <core/utils/serialize.hpp>

#include "scope.hpp"

namespace hs::scope {

using StdScope = Scope<>;

TEST(StdScopeSerialize, RoundTripPreservesModifiersAndTagLinks) {
  auto mutable_definitions = std::make_shared<ruleset::VariableDefinitions<StdBaseTypes>>();
  ASSERT_TRUE(mutable_definitions
                  ->AddNumericDefinition("numeric_var", ruleset::NumericVariableDefinition<>{})
                  .has_value());
  ASSERT_TRUE(
      mutable_definitions->AddStringDefinition("string_var", ruleset::StringVariableDefinition<>{})
          .has_value());
  std::shared_ptr<const ruleset::VariableDefinitions<StdBaseTypes>> const_definitions =
      mutable_definitions;
  ruleset::VariableDefinitionsConstPtr<StdBaseTypes> definitions{const_definitions};

  ScopePtr<StdBaseTypes> tag_scope{"tag.scope", types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS};
  StdScope source{"source.scope", types::ScopeType::SCOPE_TYPE_IMPROVEMENT};
  source.SetVariableDefinitions(definitions);

  ASSERT_TRUE(source.SetNumericModifier("numeric_var", "base", 10, 0.5, 7).has_value());
  ASSERT_TRUE(source.SetStringModifier("string_var", "base", "value", 3, 8).has_value());
  ASSERT_TRUE(source.AddTagLink("core.class", tag_scope).has_value());

  proto::scope::Scope proto_scope;
  SerializeTo(source, proto_scope);

  auto parsed = ParseFrom(proto_scope, serialize::To<StdScope>{});
  parsed.SetVariableDefinitions(definitions);

  absl::flat_hash_map<std::string, ScopePtr<StdBaseTypes>> scopes_by_id;
  scopes_by_id[tag_scope->GetId()] = tag_scope;
  parsed.RestoreTagLinks(scopes_by_id);

  ASSERT_EQ(parsed.GetId(), source.GetId());
  ASSERT_EQ(parsed.GetType(), source.GetType());
  auto parsed_numeric = parsed.GetNumericValue("numeric_var");
  auto source_numeric = source.GetNumericValue("numeric_var");
  ASSERT_TRUE(parsed_numeric.has_value());
  ASSERT_TRUE(source_numeric.has_value());
  EXPECT_EQ(*parsed_numeric, *source_numeric);

  auto parsed_string = parsed.GetStringValue("string_var");
  auto source_string = source.GetStringValue("string_var");
  ASSERT_TRUE(parsed_string.has_value());
  ASSERT_TRUE(source_string.has_value());
  EXPECT_EQ(*parsed_string, *source_string);

  auto parsed_numeric_time = parsed.GetModificationTime("numeric_var");
  auto source_numeric_time = source.GetModificationTime("numeric_var");
  ASSERT_TRUE(parsed_numeric_time.has_value());
  ASSERT_TRUE(source_numeric_time.has_value());
  EXPECT_EQ(*parsed_numeric_time, *source_numeric_time);

  auto parsed_string_time = parsed.GetModificationTime("string_var");
  auto source_string_time = source.GetModificationTime("string_var");
  ASSERT_TRUE(parsed_string_time.has_value());
  ASSERT_TRUE(source_string_time.has_value());
  EXPECT_EQ(*parsed_string_time, *source_string_time);
}

}  // namespace hs::scope
