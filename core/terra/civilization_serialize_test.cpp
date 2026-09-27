#include <gtest/gtest.h>
#include <terra/civilization.pb.h>

#include <core/utils/serialize.hpp>

#include "civilization.hpp"

namespace hs::terra {

using StdCivilization = Civilization<>;
using StdScope = scope::Scope<>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;
using ScopeType = types::ScopeType;

namespace {

scope::ScopeParseContext<StdBaseTypes> MakeScopeContext(StdCivilization& source) {
  scope::ScopeParseContext<StdBaseTypes> context;
  source.VisitScopes([&context](const auto& scope_ptr) {
    proto::scope::Scope proto_scope;
    SerializeTo(*scope_ptr, proto_scope);
    StdScopePtr parsed_scope{ParseFrom(proto_scope, serialize::To<StdScope>{})};
    context.scopes_by_id.try_emplace(parsed_scope->GetId(), parsed_scope);
  });
  for (auto& [_, scope_ptr] : context.scopes_by_id) {
    scope_ptr->RestoreTagLinks(context.scopes_by_id);
  }
  return context;
}

}  // namespace

TEST(StdCivilizationSerialize, RoundTripPreservesChildScopes) {
  StdCivilization source{"civ.alpha"};
  auto child = source.CreateChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");
  ASSERT_TRUE(child.has_value());

  proto::terra::Civilization proto_civilization;
  SerializeTo(source, proto_civilization);

  auto context = MakeScopeContext(source);
  auto parsed = ParseFrom(proto_civilization, serialize::To<StdCivilization>{}, context);

  EXPECT_EQ(parsed.GetId(), source.GetId());
  EXPECT_TRUE(parsed.HasChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha"));
  EXPECT_EQ(parsed.GetChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha")->GetParent()->GetId(),
            parsed.GetId());
}

}  // namespace hs::terra
