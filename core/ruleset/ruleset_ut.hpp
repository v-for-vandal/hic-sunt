#pragma once

#include <core/ruleset/effect.hpp>
#include <core/ruleset/variable_definition.hpp>
#include <core/types/scope_type.hpp>

namespace hs::ruleset::test {

using StdEffectDefinition = ruleset::EffectDefinition<StdBaseTypes>;
using StdEffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using StdVariableDefinitionsConstPtr = hs::ruleset::VariableDefinitionsConstPtr<StdBaseTypes>;

StdEffectDefinitionPtr MakeEffectDefinition(std::string id, std::string possible,
                                            std::string effect_code);
StdEffectDefinitionPtr MakeEffectDefinition(std::string id, hs::types::ScopeType scope_type,
                                            std::string possible, std::string effect_code);
// Created definitions will contain 'numeric_var' and 'string_var'
StdVariableDefinitionsConstPtr MakeSimpleVariableDefinitions();

}  // namespace hs::ruleset::test
