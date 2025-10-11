#include "plane.hpp"

#include <core/utils/serialize.hpp>

#include <gtest/gtest.h>

namespace hs::terra {

using namespace ::hs::geometry::literals;

using StdPlane = Plane<>;

TEST(StdPlane, Serialize) {
  StdPlane ref_plane(ControlObjectPtr{}, "test",

                     StdPlane::QRSBox(StdPlane::QRSCoords(-1_q, -2_r),
                                      StdPlane::QRSCoords(4_q, 6_r)),
                     4);

  std::string storage;

  proto::terra::Plane proto_plane;
  SerializeTo(ref_plane, proto_plane);
  proto_plane.SerializeToString(&storage);

  proto::terra::Plane proto_read_plane;
  ASSERT_TRUE(proto_read_plane.ParseFromString(storage));

  auto parse_plane = ParseFrom(proto_read_plane, serialize::To<StdPlane>{});

  EXPECT_EQ(ref_plane, parse_plane);
  EXPECT_EQ(ref_plane.GetSurfaceObject().data_size(),
            parse_plane.GetSurfaceObject().data_size());
}

} // namespace hs::terra
