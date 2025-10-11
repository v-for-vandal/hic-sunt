#include <core/geometry/coord_system.hpp>
#include <core/geometry/coords.hpp>

namespace hs::godot {

using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;

inline auto cast_qrs_size(Vector2i size) noexcept {
  auto q_size = typename QRSCoordinateSystem::QDelta{size.x};
  auto r_size = typename QRSCoordinateSystem::RDelta{size.y};

  return QRSSize{q_size, r_size};
}

inline QRSCoords cast_qrs(Vector2i coords) noexcept {
  return QRSCoords{QRSCoordinateSystem::QAxis{coords.x},
                   QRSCoordinateSystem::RAxis{coords.y}};
}

inline QRSBox cast_qrs_box(Rect2i coords) noexcept {
  return QRSBox(cast_qrs(coords.position), cast_qrs(coords.get_end()));
}

}  // namespace hs::godot
