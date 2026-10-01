#include <gtest/gtest.h>
#include <region/region.pb.h>

#include <core/utils/serialize.hpp>

#include "region.hpp"

namespace hs::region {

using StdRegion = Region<>;
using StdScope = scope::Scope<>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;

namespace {

scope::ScopeParseContext<StdBaseTypes> MakeScopeContext(StdRegion& source) {
  scope::ScopeParseContext<StdBaseTypes> context;
  source.VisitScopes([&context](const auto& scope_ptr) {
    proto::scope::Scope proto_scope;
    SerializeTo(*scope_ptr, proto_scope);
    StdScopePtr parsed_scope{ParseFrom(proto_scope, serialize::To<StdScope>{}, context)};
    context.scopes_by_id.try_emplace(parsed_scope->GetId(), parsed_scope);
  });
  return context;
}

}  // namespace

TEST(StdRegionSerialize, RoundTripPreservesPersistentStateAndScope) {
  StdRegion source{"region.alpha", 2};
  ASSERT_TRUE(source.SetCityId("city.alpha"));

  proto::region::Region proto_region;
  SerializeTo(source, proto_region);

  auto context = MakeScopeContext(source);
  auto parsed = ParseFrom(proto_region, serialize::To<StdRegion>{}, context);

  EXPECT_EQ(parsed.GetId(), source.GetId());
  EXPECT_EQ(parsed.GetCityId(), source.GetCityId());
  EXPECT_EQ(parsed.GetScope()->GetId(), source.GetScope()->GetId());
  EXPECT_EQ(parsed.GetScope()->GetType(), source.GetScope()->GetType());
  EXPECT_EQ(parsed.GetSurfaceObject().data_size(), source.GetSurfaceObject().data_size());
}

}  // namespace hs::region
