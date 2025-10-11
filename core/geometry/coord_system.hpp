#pragma once

#include <fmt/format.h>

#include <compare>

namespace hs::geometry {

template <int xt, int yt, int zt>
struct IntVectorConstant {
  static constexpr int x = xt;
  static constexpr int y = yt;
  static constexpr int z = zt;
};

using ZeroIntVectorConstant = IntVectorConstant<0, 0, 0>;
using OnesIntVectorConstant = IntVectorConstant<1, 1, 1>;

template <typename Tag>
class IntDelta {
 public:
  constexpr IntDelta() noexcept {}
  constexpr explicit IntDelta(int val) noexcept : val_(val) {}
  constexpr IntDelta(const IntDelta &) noexcept = default;
  constexpr IntDelta(IntDelta &&) noexcept = default;
  constexpr IntDelta &operator=(const IntDelta &) noexcept = default;
  constexpr IntDelta &operator=(IntDelta &&) noexcept = default;

  auto operator<=>(const IntDelta &other) const noexcept = default;

  auto operator<=>(int val) const noexcept { return val_ <=> val; }

  auto operator+(IntDelta other) const noexcept {
    return IntDelta{val_ + other.val_};
  }
  auto operator-(IntDelta other) const noexcept {
    return IntDelta{val_ + other.val_};
  }
  auto operator+=(IntDelta other) noexcept { val_ += other.val_; }
  auto operator-=(IntDelta other) noexcept { val_ -= other.val_; }
  auto operator*=(int other) noexcept { val_ *= other; }

  auto operator++() noexcept { return val_++; }
  auto operator++(int) noexcept { return val_++; }
  auto operator--() noexcept { return val_--; }
  auto operator--(int) noexcept { return val_--; }
  auto operator-() const noexcept { return IntDelta{-val_}; }

  auto operator*(int tgt) noexcept { return IntDelta(val_ * tgt); }
  auto operator/(int tgt) noexcept { return IntDelta(val_ / tgt); }

  int ToUnderlying() const noexcept { return val_; }

 private:
  int val_;
};

template <typename Tag>
class IntAxis {
 public:
  using Delta = IntDelta<Tag>;
  IntAxis() noexcept {}
  explicit IntAxis(int val) noexcept : val_(val) {}
  IntAxis(const IntAxis &) noexcept = default;
  IntAxis(IntAxis &&) noexcept = default;
  IntAxis &operator=(const IntAxis &) noexcept = default;
  IntAxis &operator=(IntAxis &&) noexcept = default;

  auto operator<=>(const IntAxis &other) const noexcept = default;

  auto operator<=>(int val) const noexcept { return val_ <=> val; }

  auto operator++() noexcept { return val_++; }
  auto operator++(int) noexcept { return val_++; }
  auto operator--() noexcept { return val_--; }
  auto operator--(int) noexcept { return val_--; }
  auto operator-() const noexcept { return IntAxis{-val_}; }

  // We do arithmetic with simple ints
  auto operator+(int tgt) const { return IntAxis(val_ + tgt); }
  auto operator-(int tgt) const { return IntAxis(val_ - tgt); }
  auto operator+(Delta tgt) const { return IntAxis(val_ + tgt.ToUnderlying()); }
  auto operator-(Delta tgt) const { return IntAxis(val_ - tgt.ToUnderlying()); }

  auto operator*(int tgt) const noexcept { return IntAxis(val_ * tgt); }
  auto operator/(int tgt) const noexcept { return IntAxis(val_ / tgt); }
  Delta operator-(IntAxis other) const { return Delta(val_ - other.val_); }

  int ToUnderlying() const noexcept { return val_; }

  Delta AsDelta() const noexcept { return Delta{val_}; }

  template <typename H>
  friend H AbslHashValue(H h, const IntAxis &a) {
    return H::combine(std::move(h), a.val_);
  }

  // both start and end are inclusive, start could be greater than end.
  bool InRange(IntAxis start, IntAxis end) const noexcept {
    return (val_ - start.val_) * (val_ - end.val_) <= 0;
  }

 private:
  int val_;
};

template <typename Tag>
IntAxis<Tag> abs(IntAxis<Tag> value) {
  return IntAxis<Tag>(std::abs(value.ToUnderlying()));
}

template <typename Tag>
IntDelta<Tag> abs(IntDelta<Tag> value) {
  return IntDelta<Tag>(std::abs(value.ToUnderlying()));
}

namespace details {

template <typename T>
struct QAxisTag {};

template <typename T>
struct RAxisTag {};

template <typename T>
struct SAxisTag {};

}  // namespace details
template <typename CoordinateSystem>
using QAxisBase = IntAxis<details::QAxisTag<CoordinateSystem>>;

template <typename CoordinateSystem>
using QDeltaBase = typename QAxisBase<CoordinateSystem>::Delta;

template <typename CoordinateSystem>
using RAxisBase = IntAxis<details::RAxisTag<CoordinateSystem>>;

template <typename CoordinateSystem>
using RDeltaBase = typename RAxisBase<CoordinateSystem>::Delta;

template <typename CoordinateSystem>
using SAxisBase = IntAxis<details::SAxisTag<CoordinateSystem>>;

template <typename CoordinateSystem>
using SDeltaBase = typename SAxisBase<CoordinateSystem>::Delta;

struct QRSCoordinateSystem {
  using QAxis = QAxisBase<QRSCoordinateSystem>;
  using RAxis = RAxisBase<QRSCoordinateSystem>;
  using SAxis = SAxisBase<QRSCoordinateSystem>;
  using QDelta = QDeltaBase<QRSCoordinateSystem>;
  using RDelta = RDeltaBase<QRSCoordinateSystem>;
  using SDelta = SDeltaBase<QRSCoordinateSystem>;
};

namespace literals {
inline QRSCoordinateSystem::QAxis operator""_q(unsigned long long int value) {
  return QRSCoordinateSystem::QAxis{static_cast<int>(value)};
}
inline QRSCoordinateSystem::RAxis operator""_r(unsigned long long int value) {
  return QRSCoordinateSystem::RAxis{static_cast<int>(value)};
}
inline QRSCoordinateSystem::SAxis operator""_s(unsigned long long int value) {
  return QRSCoordinateSystem::SAxis{static_cast<int>(value)};
}

inline QRSCoordinateSystem::QDelta operator""_dq(unsigned long long int value) {
  return QRSCoordinateSystem::QDelta{static_cast<int>(value)};
}
inline QRSCoordinateSystem::RDelta operator""_dr(unsigned long long int value) {
  return QRSCoordinateSystem::RDelta{static_cast<int>(value)};
}
inline QRSCoordinateSystem::SDelta operator""_ds(unsigned long long int value) {
  return QRSCoordinateSystem::SDelta{static_cast<int>(value)};
}

}  // namespace literals

}  // namespace hs::geometry

template <typename T>
struct fmt::formatter<::hs::geometry::IntAxis<T>> {
  constexpr auto parse(format_parse_context &ctx) const {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for axis");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(const ::hs::geometry::IntAxis<T> &v, FormatCtx &ctx) const {
    return fmt::format_to(ctx.out(), "{}", v.ToUnderlying());
  }
};

template <typename T>
struct fmt::formatter<::hs::geometry::IntDelta<T>> {
  constexpr auto parse(format_parse_context &ctx) const {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for axis");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(const ::hs::geometry::IntDelta<T> &v, FormatCtx &ctx) const {
    return fmt::format_to(ctx.out(), "{}", v.ToUnderlying());
  }
};
