#pragma once

#include <core/scope/scope.hpp>

namespace hs::scope::test {

    using ScopeType = hs::types::ScopeType;
    using StdScope = Scope<StdBaseTypes>;
    using StdScopePtr = ScopePtr<StdBaseTypes>;
    using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
    using StdVariableDefinitionsPtr =
        hs::ruleset::VariableDefinitionsPtr<StdBaseTypes>;

    /*! This function returns a simple scope with two definitions - numeric_var and
     * string_var. You should use it as root scope, so that those definitions were
     * inherited
     */
    StdScopePtr MakeSimpleScope(ScopeType type = ScopeType::SCOPE_TYPE_WORLD);

    /*! This function will create Simple scope and will additionally seed variables
     * with predefined values:
     *
     * NumericVariable { id = "numeric_var", key = "seed",  add = 2.0, mult = 0.0 }
     * StringVariable { id = "string_var", key = "seed", value = "value", level = 3.0}
     */
    StdScopePtr MakeSeededScope(ScopeType type = ScopeType::SCOPE_TYPE_WORLD);

}
