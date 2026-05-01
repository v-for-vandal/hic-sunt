#include "effect.hpp"

#include <gtest/gtest.h>

#include <ruleset/effect.pb.h>
#include <system_error>

namespace hs::ruleset {

using StdEffectDefinition = EffectDefinition<StdBaseTypes>;

namespace {

proto::ruleset::effect::Effect MakeEffect(std::string id, std::string lua) {
  proto::ruleset::effect::Effect effect;
  effect.set_id(std::move(id));
  effect.mutable_code()->set_lua(std::move(lua));
  return effect;
}

}  // namespace

TEST(StdEffectDefinition, StoresIdAndProcessedCode) {
  StdEffectDefinition effect(MakeEffect(
      "sample.effect",
      "return VAR(foo.bar) .. ':' .. VAR(\"baz_qux\")"));

  EXPECT_EQ(effect.GetId(), "sample.effect");
  EXPECT_EQ(effect.GetCode(), "return \"foo.bar\" .. ':' .. \"baz_qux\"");

  ASSERT_EQ(effect.GetDependencies().size(), 2u);
  EXPECT_EQ(effect.GetDependencies()[0], "foo.bar");
  EXPECT_EQ(effect.GetDependencies()[1], "baz_qux");
}

TEST(StdEffectDefinition, IgnoresVarInsideDoubleQuotedStrings) {
  StdEffectDefinition effect(MakeEffect(
      "sample.effect",
      "return \"VAR(fake.value)\", VAR(real_value)"));

  EXPECT_EQ(effect.GetCode(), "return \"VAR(fake.value)\", \"real_value\"");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0], "real_value");
}

TEST(StdEffectDefinition, IgnoresVarInsideSingleQuotedStrings) {
  StdEffectDefinition effect(MakeEffect(
      "sample.effect",
      "return 'VAR(fake.value)', VAR(real_value)"));

  EXPECT_EQ(effect.GetCode(), "return 'VAR(fake.value)', \"real_value\"");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0], "real_value");
}

TEST(StdEffectDefinition, IgnoresVarInsideLineComments) {
  StdEffectDefinition effect(MakeEffect(
      "sample.effect",
      "-- VAR(fake.value)\nreturn VAR(real_value)"));

  EXPECT_EQ(effect.GetCode(), "-- VAR(fake.value)\nreturn \"real_value\"");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0], "real_value");
}

TEST(StdEffectDefinition, IgnoresVarInsideBlockComments) {
  StdEffectDefinition effect(MakeEffect(
      "sample.effect",
      "--[[ VAR(fake.value) ]]\nreturn VAR(real_value)"));

  EXPECT_EQ(effect.GetCode(), "--[[ VAR(fake.value) ]]\nreturn \"real_value\"");
  ASSERT_EQ(effect.GetDependencies().size(), 1u);
  EXPECT_EQ(effect.GetDependencies()[0], "real_value");
}

TEST(StdEffectDefinition, RejectsInvalidIdentifierCharacters) {
  EXPECT_THROW(
      StdEffectDefinition(MakeEffect("sample.effect", "return VAR(\"a+z\")")),
      std::system_error);
}

TEST(StdEffectDefinition, RejectsPartiallyQuotedIdentifiers) {
  EXPECT_THROW(
      StdEffectDefinition(MakeEffect("sample.effect", "return VAR(\"ab\"c)")),
      std::system_error);
}

TEST(StdEffectDefinition, RejectsUnclosedVarCall) {
  EXPECT_THROW(
      StdEffectDefinition(MakeEffect("sample.effect", "return VAR(abc")),
      std::system_error);
}

}  // namespace hs::ruleset
