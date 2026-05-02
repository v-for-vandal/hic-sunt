#include "effect_instance.hpp"

#include <gtest/gtest.h>

#include <ruleset/effect.pb.h>

namespace hs::session {

using StdEffectDefinition = ruleset::EffectDefinition<StdBaseTypes>;
using StdEffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<StdBaseTypes>;
using StdEffectInstance = EffectInstance<StdBaseTypes>;

namespace {

StdEffectDefinitionPtr MakeEffectDefinition(std::string id, std::string lua) {
  proto::ruleset::effect::Effect effect;
  effect.set_id(std::move(id));
  effect.mutable_code()->set_lua(std::move(lua));

  auto definition = std::make_shared<StdEffectDefinition>(std::move(effect));
  return StdEffectDefinitionPtr(
      std::static_pointer_cast<const StdEffectDefinition>(definition));
}

}  // namespace

TEST(StdEffectInstance, ExposesDefinitionIdentityAndProcessedCode) {
  auto definition = MakeEffectDefinition("effect.id", "return VAR(foo)");

  StdEffectInstance instance(definition);

  EXPECT_EQ(instance.GetId(), "effect.id");
  EXPECT_EQ(instance.GetCode(), "return \"foo\"");
  EXPECT_EQ(instance.GetDefinition()->GetId(), "effect.id");
}

TEST(StdEffectInstance, LoadsLuaChunkIntoState) {
  auto definition = MakeEffectDefinition("effect.id", "return 41 + 1");

  StdEffectInstance instance(definition);

  ASSERT_TRUE(instance.GetChunk().valid());
  sol::protected_function_result result = instance.GetChunk()();
  ASSERT_TRUE(result.valid());

  int value = result;
  EXPECT_EQ(value, 42);
}

}  // namespace hs::session
