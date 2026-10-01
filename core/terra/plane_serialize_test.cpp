#include <gtest/gtest.h>
#include <terra/plane.pb.h>

#include <core/utils/serialize.hpp>

#include "plane.hpp"

namespace hs::terra {

using namespace ::hs::geometry::literals;
using StdPlane = Plane<>;
using StdScope = scope::Scope<>;
using StdScopePtr = scope::ScopePtr<StdBaseTypes>;

namespace {

scope::ScopeParseContext<StdBaseTypes> MakeScopeContext(StdPlane& source) {
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

TEST(StdPlaneSerialize, RoundTripPreservesPersistentStateAndIndexes) {
  StdPlane source(ControlObjectPtr{}, "plane.alpha",
                  StdPlane::QRSBox(StdPlane::QRSCoords(-1_q, -1_r), StdPlane::QRSCoords(1_q, 1_r)),
                  1, 2);

  proto::terra::Plane proto_plane;
  SerializeTo(source, proto_plane);

  auto context = MakeScopeContext(source);
  auto parsed = ParseFrom(proto_plane, serialize::To<StdPlane>{}, context);

  EXPECT_EQ(parsed.GetPlaneId(), source.GetPlaneId());
  EXPECT_EQ(parsed.GetExternalRadius(), source.GetExternalRadius());
  EXPECT_EQ(parsed.GetScope()->GetId(), source.GetScope()->GetId());
  EXPECT_EQ(parsed.GetSurfaceObject().data_size(), source.GetSurfaceObject().data_size());
  EXPECT_EQ(parsed.GetRegions().size(), source.GetRegions().size());
}

}  // namespace hs::terra
