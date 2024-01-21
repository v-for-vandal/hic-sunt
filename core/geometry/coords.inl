#pragma once

namespace geometry {

template<typename S>
static const Coords<S> kUndefinedCoords{
  typename S::QAXis{std::numeric_limits<int>::max()},
                                     typename S::RAxis{std::numeric_limits<int>::max()},
                                     typename S::SAxis{std::numeric_limits<int>::max()}};

template<typename T>
inline bool Coords<T>::IsUndefined() const noexcept {
  return *this == kUndefinedCoords<T>;
}

template<typename T>
inline void Coords<T>::SetUndefined() noexcept { *this = kUndefinedCoords<T>; }

template<typename T>
inline Coords<T> Coords<T>::operator+(const DeltaCoords& second) const noexcept {
  if (IsUndefined()) {
    return kUndefinedCoords<T>;
  }

  return Coords{q() + second.q(), r() + second.r()};
}

template<typename T>
inline DeltaCoords<T> Coords<T>::operator-(const Coords<T>& second) const noexcept {
  if (IsUndefined() || second.IsUndefined()) {
    return DeltaCoords<T>::GetUndefinedDelta();
  }

  return DeltaCoords{
      (q() - second.q()), .r() = (r() - second.r())};
}


}

