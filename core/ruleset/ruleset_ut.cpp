#include "ruleset_ut.hpp"
#include "core/types/scope_type.hpp"

namespace hs::ruleset::test {

StdEffectDefinitionPtr MakeEffectDefinition(std::string id,
                                            types::ScopeType scope_type,
                                            std::string possible_code,
                                            std::string effect_code) {
  proto::ruleset::effect::Effect effect;
  effect.set_id(std::move(id));
  effect.mutable_possible()->set_lua(std::move(possible_code));
  effect.mutable_effect()->set_lua(std::move(effect_code));
  effect.set_scope_type(scope_type);

  auto definition = std::make_shared<StdEffectDefinition>(std::move(effect));
  return StdEffectDefinitionPtr(
      std::static_pointer_cast<const StdEffectDefinition>(definition));
}

StdEffectDefinitionPtr MakeEffectDefinition(std::string id,
                                            std::string possible_code,
                                            std::string effect_code) {
                                                return MakeEffectDefinition(id, types::ScopeType::SCOPE_TYPE_WORLD, possible_code, effect_code);
                                            }

}
