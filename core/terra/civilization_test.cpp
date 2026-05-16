#include "civilization.hpp"

#include <gtest/gtest.h>

#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/scope/scope_ut.hpp>

namespace hs::terra {

using ScopeType = types::ScopeType;
using StdCivilization = Civilization<>;
using StdScope = scope::Scope<>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;

namespace {

StdScopePtr MakeScope(std::string_view id, ScopeType type) {
    return scope::test::MakeSimpleScope(type, id);
}

}  // namespace

TEST(StdCivilization, AddChildScopeStoresValidChild) {
  StdCivilization civilization{"civ.test"};
  auto city_scope = MakeScope("city.alpha", ScopeType::SCOPE_TYPE_CITY);

  auto result = civilization.AddChildScope(city_scope);

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(civilization.HasChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha"));

  StdScopePtr stored_scope = civilization.GetChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");
  EXPECT_EQ(stored_scope, city_scope);
  EXPECT_EQ(stored_scope->GetParent(), civilization.GetScope());

  const auto* scopes_by_type = civilization.FindScopesByType(ScopeType::SCOPE_TYPE_CITY);
  ASSERT_NE(scopes_by_type, nullptr);
  auto fit = scopes_by_type->find("city.alpha");
  ASSERT_NE(fit, scopes_by_type->end());
  EXPECT_EQ(fit->second, city_scope);
}

TEST(StdCivilization, AddChildScopeRejectsDisallowedType) {
  StdCivilization civilization{"civ.test"};
  auto region_scope = MakeScope("region.alpha", ScopeType::SCOPE_TYPE_REGION);

  auto result = civilization.AddChildScope(region_scope);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  EXPECT_FALSE(civilization.HasChildScope(ScopeType::SCOPE_TYPE_REGION, "region.alpha"));

  auto returned_scope = civilization.GetChildScope(ScopeType::SCOPE_TYPE_REGION, "region.alpha");
  EXPECT_NE(returned_scope, nullptr);
  EXPECT_EQ(returned_scope->GetParent(), nullptr);
}

TEST(StdCivilization, AddChildScopeRejectsDifferentPointerWithSameId) {
  StdCivilization civilization{"civ.test"};
  auto first_city_scope = MakeScope("city.alpha", ScopeType::SCOPE_TYPE_CITY);
  auto second_city_scope = MakeScope("city.alpha", ScopeType::SCOPE_TYPE_CITY);

  ASSERT_TRUE(civilization.AddChildScope(first_city_scope).has_value());

  auto result = civilization.AddChildScope(second_city_scope);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_SCOPE_ALREADY_EXISTS);

  auto stored_scope = civilization.GetChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");
  EXPECT_EQ(stored_scope, first_city_scope);
  EXPECT_EQ(stored_scope->GetParent(), civilization.GetScope());
}

TEST(StdCivilization, CreateChildScopeCreatesAndParentsScope) {
  StdCivilization civilization{"civ.test"};

  auto result = civilization.CreateChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(civilization.HasChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha"));
  EXPECT_EQ((*result)->GetType(), ScopeType::SCOPE_TYPE_CITY);
  EXPECT_EQ((*result)->GetId(), "city.alpha");
  EXPECT_EQ((*result)->GetParent(), civilization.GetScope());

  auto stored_scope = civilization.GetChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");
  EXPECT_EQ(stored_scope, *result);
  EXPECT_EQ(stored_scope->GetParent(), civilization.GetScope());
}

TEST(StdCivilization, CreateChildScopeRejectsDisallowedType) {
  StdCivilization civilization{"civ.test"};

  auto result = civilization.CreateChildScope(ScopeType::SCOPE_TYPE_REGION, "region.alpha");

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  EXPECT_FALSE(civilization.HasChildScope(ScopeType::SCOPE_TYPE_REGION, "region.alpha"));

  auto returned_scope = civilization.GetChildScope(ScopeType::SCOPE_TYPE_REGION, "region.alpha");
  EXPECT_NE(returned_scope, nullptr);
  EXPECT_EQ(returned_scope->GetParent(), nullptr);
}

TEST(StdCivilization, GetOrCreateChildScopeReturnsExistingObject) {
  StdCivilization civilization{"civ.test"};
  auto created = civilization.CreateChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");
  ASSERT_TRUE(created.has_value());

  auto result = civilization.GetOrCreateChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, *created);
  EXPECT_EQ((*result)->GetParent(), civilization.GetScope());
}

TEST(StdCivilization, GetOrCreateChildScopeCreatesMissingObject) {
  StdCivilization civilization{"civ.test"};

  auto result = civilization.GetOrCreateChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(civilization.HasChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha"));
  EXPECT_EQ((*result)->GetId(), "city.alpha");
  EXPECT_EQ((*result)->GetType(), ScopeType::SCOPE_TYPE_CITY);
  EXPECT_EQ((*result)->GetParent(), civilization.GetScope());
}

TEST(StdCivilization, GetOrCreateChildScopeRejectsDisallowedType) {
  StdCivilization civilization{"civ.test"};

  auto result = civilization.GetOrCreateChildScope(ScopeType::SCOPE_TYPE_REGION, "region.alpha");

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  EXPECT_FALSE(civilization.HasChildScope(ScopeType::SCOPE_TYPE_REGION, "region.alpha"));
}

TEST(StdCivilization, GetChildScopeReturnsAdHocScopeForMissingObject) {
  StdCivilization civilization{"civ.test"};

  auto missing_scope = civilization.GetChildScope(ScopeType::SCOPE_TYPE_CITY, "city.missing");

  EXPECT_NE(missing_scope, nullptr);
  EXPECT_EQ(missing_scope->GetParent(), nullptr);
}

}  // namespace hs::terra
