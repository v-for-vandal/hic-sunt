#include <core/geometry/surface_shape.hpp>

namespace hs::geometry {

namespace {

// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

}

HexagonSurface ParseFrom(const proto::geometry::HexagonSurface& source, serialize::To<HexagonSurface>) {

}

HexagonSurface SerializeTo(const HexagonSurface& source, const proto::geometry::HexagonSurface& target) {

}

RhombusSurface ParseFrom(const proto::geometry::RhombusSurface& source, serialize::To<RhombusSurface>) {

}

RhombusSurface SerializeTo(const RhombusSurface& source, const proto::geometry::RhombusSurface& target) {

}

SurfaceShape<geometry::QRSCoordinateSystem> ParseFrom(const proto::geometry::SurfaceShape& source, serialize::To<SurfaceShape<geometry::QRSCoordinateSystem>>)
{
    SurfaceShape<geometry::QRSCoordinateSystem> result;

    if (source.has_hexagon()) {
        result = SurfaceShape(
            ParseFrom(source.hexagon(), serialize::To<HexagonSurface>{});
            );
    } else if (source.has_rhombus()) {
        result = SurfaceShape(
            ParseFrom(source.hexagon(), serialize::To<RhombusSurface>{});
            );
    }

    return result;
}

void SerializeTo(const SurfaceShape<geometry::QRSCoordinateSystem>& source, proto::geometry::SurfaceShape& target)
{
    auto visitor = overloads {
        [&target](const HexagonSurface& surface) {
            SerializeTo(surface, *target.mutable_hexagon());
        },
        [&target](const RhombusSurface& surface) {
            SerializeTo(surface, *target.mutable_rhombus());
        }
    };

    data_.visit(visitor);
}

}
