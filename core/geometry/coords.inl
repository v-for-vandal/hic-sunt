#pragma once

namespace geometry {

template<typename S>
static const Coords<S> kUndefinedCoords{
  typename S::XAxis{std::numeric_limits<int>::max()},
                                     typename S::YAxis{std::numeric_limits<int>::max()},
                                     typename S::ZAxis{std::numeric_limits<int>::max()}};

template<typename T>
inline bool Coords<T>::IsUndefined() const noexcept {
  return *this == kUndefinedCoords<T>;
}

template<typename T>
inline Coords<T> Coords<T>::operator+(const DeltaCoords& second) const noexcept {
  if (IsUndefined()) {
    return kUndefinedCoords<T>;
  }

  return Coords{.x = x + second.x, .y = y + second.y, .z = z + second.z};
}

template<typename T>
inline DeltaCoords<T> Coords<T>::operator-(const Coords<T>& second) const noexcept {
  if (IsUndefined() || second.IsUndefined()) {
    return DeltaCoords{};
  }

  return DeltaCoords{
      .x = (x - second.x), .y = (y - second.y), .z = (z - second.z)};
}

template<typename T>
inline Coords<T> Coords<T>::operator+(Direction direction) const noexcept {
  if (IsUndefined()) {
    return kUndefinedCoords<T>;
  }
  switch (direction) {
    case Direction::kXNegative:
      return Coords{.x = x - 1, .y = y, .z = z};
    case Direction::kXPositive:
      return Coords{.x = x + 1, .y = y, .z = z};
    case Direction::kYNegative:
      return Coords{.x = x, .y = y - 1, .z = z};
    case Direction::kYPositive:
      return Coords{.x = x, .y = y + 1, .z = z};
    case Direction::kZNegative:
      return Coords{.x = x, .y = y, .z = z - 1};
    case Direction::kZPositive:
      return Coords{.x = x, .y = y, .z = z + 1};
    default:
      // TODO: Mark unreachable
      CHECK(false);
      std::unreachable();
  }

  return *this;
}

template <typename S>
inline Coords<RawCoordinateSystem> Coords<S>::ToRaw() const {
  using RawCoords = Coords<RawCoordinateSystem>;
  if( IsUndefined()) {
    return kUndefinedCoords<RawCoordinateSystem>;
  }
  return RawCoords{RawCoords::XAxis{S::transform_x(x.ToUnderlying())},
                   RawCoords::YAxis{S::transform_y(y.ToUnderlying())},
                   RawCoords::ZAxis{S::transform_z(z.ToUnderlying())}};
}

}

