#include "scope_ut.hpp"

#include <gtest/gtest.h>

#include <core/ruleset/ruleset_ut.hpp>

namespace hs::scope::test {

    StdScopePtr MakeSimpleScope(ScopeType scope_type) {
      StdScopePtr scope("test_scope", scope_type);
      scope->SetVariableDefinitions(hs::ruleset::test::MakeSimpleVariableDefinitions());
      return scope;
    }

    StdScopePtr MakeSeededScope(ScopeType type ) {
        StdScopePtr scope = MakeSimpleScope(type);
        EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 2.0, 0.0));
        EXPECT_TRUE(scope->SetStringModifier("string_var", "seed", "value", 1.0));
        return scope;
    }

}
