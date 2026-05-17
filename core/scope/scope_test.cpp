#include "scope.hpp"

#include <gtest/gtest.h>

#include <core/scope/scope_ut.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

namespace hs::scope {

using StdScope = Scope<StdBaseTypes>;
using StdScopePtr = ScopePtr<StdBaseTypes>;
using StdVariableDefinitions = hs::ruleset::VariableDefinitions<StdBaseTypes>;
using StdVariableDefinitionsPtr = hs::ruleset::VariableDefinitionsPtr<StdBaseTypes>;

TEST(StdScope, Create) {
  StdScope stack_scope;

  StdScopePtr ptr_scope;
}

TEST(StdScope, NumericVariable) {
  auto scope = test::MakeSimpleScope();

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "some_key", 1.0, 2.0, 0));
  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 3.0);  // add=1.0 * (1 + mult=2.0)
}

TEST(StdScope, InheritanceParent) {
  StdScopePtr parent_scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_WORLD);
  StdScopePtr scope("test", types::ScopeType::SCOPE_TYPE_PLANE);

  ASSERT_TRUE(scope->SetParent(parent_scope));

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "some_key", 1.0, 1.0, 0));
  EXPECT_TRUE(parent_scope->SetNumericModifier("numeric_var", "other_key", 1.0, 1.0));
  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 6.0);  // add=(1+1) * (1 + mult=(1+1))
}

TEST(StdScope, InheritanceIncludesTagScopes) {
  StdScopePtr parent_scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_WORLD, "parent");
  StdScopePtr tag_scope("tag", types::ScopeType::SCOPE_TYPE_WORLD);
  StdScopePtr scope("test", types::ScopeType::SCOPE_TYPE_PLANE);

  ASSERT_TRUE(scope->SetParent(parent_scope));
  ASSERT_TRUE(scope->AddTagLink("tag_name", tag_scope));

  EXPECT_TRUE(scope->SetNumericModifier("numeric_var", "self", 1.0, 0.0));
  EXPECT_TRUE(parent_scope->SetNumericModifier("numeric_var", "parent", 2.0, 1.0));
  EXPECT_TRUE(tag_scope->SetNumericModifier("numeric_var", "tag", 3.0, 0.5));

  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 12.0);  // add=(1+2+3) * (1 + mult=(0+1+0.5))
}

TEST(StdScope, GraphTraversalSkipsAlreadyVisitedTagScopes) {
  StdScopePtr shared_scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_WORLD, "shared");
  StdScopePtr parent_scope("parent", types::ScopeType::SCOPE_TYPE_WORLD);
  StdScopePtr scope("test", types::ScopeType::SCOPE_TYPE_PLANE);

  ASSERT_TRUE(scope->SetParent(parent_scope));
  ASSERT_TRUE(scope->AddTagLink("tag_name", shared_scope));
  ASSERT_TRUE(parent_scope->AddTagLink("tag_name", shared_scope));

  EXPECT_TRUE(shared_scope->SetNumericModifier("numeric_var", "shared", 2.0, 1.0));

  auto result = scope->GetNumericValue("numeric_var");
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, 4.0);  // shared scope contributes only once

  struct NumericExplanation {
    std::string scope_id;
    std::string modifier;
    double add{0};
    double mult{0};
  };

  std::vector<NumericExplanation> explanations;
  scope->ExplainNumericVariable("numeric_var",
                                [&explanations](const auto& scope_id, const auto&, const auto& modifier,
                                                auto add, auto mult) {
                                  explanations.push_back(NumericExplanation{
                                      .scope_id = scope_id,
                                      .modifier = modifier,
                                      .add = add,
                                      .mult = mult,
                                  });
                                });

  ASSERT_EQ(explanations.size(), 1u);
  EXPECT_EQ(explanations[0].scope_id, "shared");
  EXPECT_EQ(explanations[0].modifier, "shared");

  auto modification_time = scope->GetModificationTime("numeric_var");
  ASSERT_TRUE(modification_time.has_value());
  EXPECT_EQ(*modification_time, 0u);
}

TEST(StdScope, AddTagLinkRejectsDuplicateScopeId) {
  StdScopePtr scope = test::MakeSimpleScope(types::ScopeType::SCOPE_TYPE_WORLD, "scope");
  StdScopePtr tag_scope_a("shared", types::ScopeType::SCOPE_TYPE_WORLD);
  StdScopePtr tag_scope_b("shared", types::ScopeType::SCOPE_TYPE_WORLD);

  ASSERT_TRUE(scope->AddTagLink("tag_a", tag_scope_a));

  auto result = scope->AddTagLink("tag_b", tag_scope_b);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
}

}  // namespace hs::scope
