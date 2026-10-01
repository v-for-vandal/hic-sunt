#include "effect.hpp"

#include <gtest/gtest.h>
#include <ruleset/effect.pb.h>

namespace hs::ruleset {

using StdEffectDefinition = EffectDefinition<StdBaseTypes>;

namespace {

proto::ruleset::effect::Effect MakeEffect(std::string id, std::string possible,
                                          std::string effect_code) {
  proto::ruleset::effect::Effect effect;
  effect.set_id(std::move(id));
  effect.mutable_possible()->set_lua(std::move(possible));
  effect.mutable_effect()->set_lua(std::move(effect_code));
  return effect;
}

}  // namespace

TEST(StdEffectDefinition, StoresIdAndProcessedCode) {
  StdEffectDefinition effect(MakeEffect("sample.effect", "return VAR(possible.dep)",
                                        "return VAR(foo.bar) .. ':' .. VAR(\"baz_qux\")"));

  EXPECT_EQ(effect.GetId(), "sample.effect");
  ASSERT_TRUE(effect.GePossibleCode().has_value());
  EXPECT_EQ(effect.GePossibleCode()->code,
            "function __hic_sunt_possible(target)\nreturn __var_0\nend");
  EXPECT_EQ(effect.GetEffectCode().code,
            "function __hic_sunt_effect(target)\nreturn __var_1 .. ':' .. "
            "__var_2\nend");

  ASSERT_EQ(effect.GetDependencies().size(), 3u);
  EXPECT_EQ(effect.GetDependencies()[0].raw_id, "possible.dep");
  EXPECT_EQ(effect.GetDependencies()[1].raw_id, "foo.bar");
  EXPECT_EQ(effect.GetDependencies()[2].raw_id, "baz_qux");

  ASSERT_EQ(effect.GetLuaVariables().size(), 3u);
  EXPECT_EQ(effect.GetLuaVariables()[0].lua_name, "__var_0");
  EXPECT_EQ(effect.GetLuaVariables()[0].parsed_query.raw_id, "possible.dep");
  EXPECT_EQ(effect.GetLuaVariables()[1].lua_name, "__var_1");
  EXPECT_EQ(effect.GetLuaVariables()[1].parsed_query.raw_id, "foo.bar");
  EXPECT_EQ(effect.GetLuaVariables()[2].lua_name, "__var_2");
  EXPECT_EQ(effect.GetLuaVariables()[2].parsed_query.raw_id, "baz_qux");
}

TEST(StdEffectDefinition, ParsesParameterizedWildcardVar) {
  StdEffectDefinition effect(
      MakeEffect("sample.effect", "return true", "return VAR(job/@*/consumes/@resource.wood)"));

  EXPECT_FALSE(effect.IsBroken());
  EXPECT_EQ(effect.GetEffectCode().code, "function __hic_sunt_effect(target)\nreturn __var_0\nend");
  ASSERT_EQ(effect.GetLuaVariables().size(), 1u);
  const auto& query = effect.GetLuaVariables()[0].parsed_query;
  EXPECT_EQ(query.raw_id, "job/@*/consumes/@resource.wood");
  EXPECT_EQ(query.normalized_id, "job/{}/consumes/{}");
  EXPECT_TRUE(query.is_parameterized);
  EXPECT_TRUE(query.has_wildcard);
  ASSERT_EQ(query.arguments.size(), 2u);
  EXPECT_EQ(query.arguments[0].kind, ParsedVariableQueryArgumentKind::kWildcard);
  EXPECT_EQ(query.arguments[1].value, "resource.wood");
}

TEST(StdEffectDefinition, ReportsNotBrokenForValidLua) {
  StdEffectDefinition effect(MakeEffect("sample.effect", "return true", "return"));

  EXPECT_FALSE(effect.IsBroken());
  EXPECT_TRUE(effect.GetLuaErrors().empty());
}

TEST(StdEffectDefinition, MarksBrokenAndCollectsErrorsForInvalidLua) {
  StdEffectDefinition effect(
      MakeEffect("sample.effect", "this is not valid lua", "also not valid lua"));

  EXPECT_TRUE(effect.IsBroken());
  ASSERT_EQ(effect.GetLuaErrors().size(), 2u);
}

TEST(StdEffectDefinition, IgnoresVarInsideDoubleQuotedStrings) {
  StdEffectDefinition effect(
      MakeEffect("sample.effect", "return true", "return \"VAR(fake.value)\", VAR(real_value)"));

  EXPECT_EQ(effect.GetEffectCode().code,
            "function __hic_sunt_effect(target)\nreturn \"VAR(fake.value)\", "
            "__var_0\nend");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0].raw_id, "real_value");
}

TEST(StdEffectDefinition, IgnoresVarInsideSingleQuotedStrings) {
  StdEffectDefinition effect(
      MakeEffect("sample.effect", "return true", "return 'VAR(fake.value)', VAR(real_value)"));

  EXPECT_EQ(effect.GetEffectCode().code,
            "function __hic_sunt_effect(target)\nreturn 'VAR(fake.value)', "
            "__var_0\nend");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0].raw_id, "real_value");
}

TEST(StdEffectDefinition, IgnoresVarInsideLineComments) {
  StdEffectDefinition effect(
      MakeEffect("sample.effect", "return true", "-- VAR(fake.value)\nreturn VAR(real_value)"));

  EXPECT_EQ(effect.GetEffectCode().code,
            "function __hic_sunt_effect(target)\n-- VAR(fake.value)\nreturn "
            "__var_0\nend");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0].raw_id, "real_value");
}

TEST(StdEffectDefinition, IgnoresVarInsideBlockComments) {
  StdEffectDefinition effect(MakeEffect("sample.effect", "return true",
                                        "--[[ VAR(fake.value) ]]\nreturn VAR(real_value)"));

  EXPECT_EQ(effect.GetEffectCode().code,
            "function __hic_sunt_effect(target)\n--[[ VAR(fake.value) "
            "]]\nreturn __var_0\nend");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0].raw_id, "real_value");
}

TEST(StdEffectDefinition, RejectsInvalidIdentifierCharacters) {
  EXPECT_TRUE(StdEffectDefinition(MakeEffect("sample.effect", "return true", "return VAR(\"a+z\")"))
                  .IsBroken());
}

TEST(StdEffectDefinition, RejectsPartiallyQuotedIdentifiers) {
  EXPECT_TRUE(StdEffectDefinition(MakeEffect("sample.effect", "return true", "return VAR(\"ab\"c)"))
                  .IsBroken());
}

TEST(StdEffectDefinition, RejectsUnclosedVarCall) {
  EXPECT_TRUE(
      StdEffectDefinition(MakeEffect("sample.effect", "return true", "return VAR(abc")).IsBroken());
}

}  // namespace hs::ruleset
