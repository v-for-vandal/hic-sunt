#include "scope_ut.hpp"

#include <gtest/gtest.h>

namespace hs::scope::test {

    namespace {
        StdVariableDefinitionsPtr MakeVariableDefinitions() {
        auto mutable_definitions = std::make_shared<StdVariableDefinitions>();
        mutable_definitions->AddNumericDefinition("numeric_var", {});
        mutable_definitions->AddStringDefinition("string_var", {});

        StdVariableDefinitionsPtr result;
        return StdVariableDefinitionsPtr(
            std::static_pointer_cast<const StdVariableDefinitions>(
                mutable_definitions));
        }
    }

    StdScopePtr MakeSimpleScope(ScopeType scope_type) {
      StdScopePtr scope("test_scope", scope_type);
      scope->SetVariableDefinitions(MakeVariableDefinitions());
      return scope;
    }

    StdScopePtr MakeSeededScope(ScopeType type ) {
        StdScopePtr scope = MakeSimpleScope(type);
        EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "seed", 2.0, 0.0));
        EXPECT_TRUE(scope->SetStringModifier("string_var", "seed", "value", 1.0));
        return scope;
    }

}
