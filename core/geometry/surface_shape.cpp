#include <core/geometry/surface_shape.hpp>

namespace hs::geometry {

namespace {

// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

}

HexagonSurface ParseFrom(const proto::geometry::HexagonSurface& source, serialize::To<HexagonSurface>) {
    return HexagonSurface(source.radius());

}

void SerializeTo(const HexagonSurface& source, proto::geometry::HexagonSurface& target) {
    target.set_radius(source.radius_);
}

RhombusSurface ParseFrom(const proto::geometry::RhombusSurface& source, serialize::To<RhombusSurface>) {
    using Coords = RhombusSurface::Coords;

    Coords start{
        RhombusSurface::CoordinateSystem::QAxis{source.q_start()},
        RhombusSurface::CoordinateSystem::RAxis{source.r_start()},
        };
    Coords end{
        RhombusSurface::CoordinateSystem::QAxis{source.q_end()},
        RhombusSurface::CoordinateSystem::RAxis{source.r_end()},
        };

    return RhombusSurface{
        RhombusSurface::Box{start, end}};

}

void SerializeTo(const RhombusSurface& source, proto::geometry::RhombusSurface& target) {

    target.set_q_start(source.BoundingBox().start().q().ToUnderlying());
    target.set_r_start(source.BoundingBox().start().r().ToUnderlying());
    target.set_q_end(source.BoundingBox().end().q().ToUnderlying());
    target.set_r_end(source.BoundingBox().end().r().ToUnderlying());
}

SurfaceShape<geometry::QRSCoordinateSystem> ParseFrom(const proto::geometry::SurfaceShape& source, serialize::To<SurfaceShape<geometry::QRSCoordinateSystem>>)
{
    if (source.has_hexagon()) {
        const HexagonSurface hexagon = ParseFrom(source.hexagon(), serialize::To<HexagonSurface>{});
        return SurfaceShape<geometry::QRSCoordinateSystem>{
            hexagon
            };
    } else if (source.has_rhombus()) {
        return SurfaceShape<geometry::QRSCoordinateSystem>(
            ParseFrom(source.rhombus(), serialize::To<RhombusSurface>{})
            );
    }

    return {Box<geometry::QRSCoordinateSystem>::MakeOne()};
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

    std::visit(visitor, source.data_);
}

}
