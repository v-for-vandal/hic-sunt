#pragma once

#include <compare>
#include <fmt/format.h>

namespace geometry {

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
  constexpr IntDelta(const IntDelta&) noexcept = default;
  constexpr IntDelta(IntDelta&&) noexcept = default;
  constexpr IntDelta& operator=(const IntDelta&) noexcept = default;
  constexpr IntDelta& operator=(IntDelta&&) noexcept = default;

  auto operator<=>(const IntDelta& other) const noexcept = default;

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
  IntAxis(const IntAxis&) noexcept = default;
  IntAxis(IntAxis&&) noexcept = default;
  IntAxis& operator=(const IntAxis&) noexcept = default;
  IntAxis& operator=(IntAxis&&) noexcept = default;

  auto operator<=>(const IntAxis& other) const noexcept = default;

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

 private:
  int val_;
};

namespace details {

template <typename T>
struct XAxisTag {};

template <typename T>
struct YAxisTag {};

template <typename T>
struct ZAxisTag {};

}  // namespace details
template <typename CoordinateSystem>
using XAxisBase = IntAxis<details::XAxisTag<CoordinateSystem>>;

template <typename CoordinateSystem>
using XDeltaBase = typename XAxisBase<CoordinateSystem>::Delta;

template <typename CoordinateSystem>
using YAxisBase = IntAxis<details::YAxisTag<CoordinateSystem>>;

template <typename CoordinateSystem>
using YDeltaBase = typename YAxisBase<CoordinateSystem>::Delta;

template <typename CoordinateSystem>
using ZAxisBase = IntAxis<details::ZAxisTag<CoordinateSystem>>;

template <typename CoordinateSystem>
using ZDeltaBase = typename ZAxisBase<CoordinateSystem>::Delta;

template <class TranslationVector, class ScaleVector>
struct CoordSystem {
  /*
  using TranslationVector = TranslationVector;
  using ScaleVector = ScaleVector;
  */
#define TRANSFORM_MACRO(a) \
  static int transform_ ## a(int val) noexcept { \
    return TranslationVector::  a + ScaleVector::  a * val;\
  }

#define REVERSE_TRANSFORM_MACRO(a) \
  static std::optional<int> reverse_transform_ ## a(int val) noexcept {\
    if (val % ScaleVector:: a != 0) {\
      return std::nullopt;\
    }\
    return val / ScaleVector::  a - TranslationVector::  a;\
  }

  TRANSFORM_MACRO(x)
  TRANSFORM_MACRO(y)
  TRANSFORM_MACRO(z)
  REVERSE_TRANSFORM_MACRO(x)
  REVERSE_TRANSFORM_MACRO(y)
  REVERSE_TRANSFORM_MACRO(z)

#undef TRANSFORM_MACRO
#undef REVERSE_TRANSFORM_MACRO

  using XAxis = XAxisBase<CoordSystem>;
  using YAxis = YAxisBase<CoordSystem>;
  using ZAxis = ZAxisBase<CoordSystem>;
  using XDelta = XDeltaBase<CoordSystem>;
  using YDelta = YDeltaBase<CoordSystem>;
  using ZDelta = ZDeltaBase<CoordSystem>;
};


using RawCoordinateSystem =
    ::geometry::CoordSystem<::geometry::ZeroIntVectorConstant, ::geometry::OnesIntVectorConstant>;

namespace literals {
  inline RawCoordinateSystem::XAxis operator"" _x(unsigned long long int value) {
    return RawCoordinateSystem::XAxis{static_cast<int>(value)};
  }
  inline RawCoordinateSystem::YAxis operator"" _y(unsigned long long int value) {
    return RawCoordinateSystem::YAxis{static_cast<int>(value)};
  }
  inline RawCoordinateSystem::ZAxis operator"" _z(unsigned long long int value) {
    return RawCoordinateSystem::ZAxis{static_cast<int>(value)};
  }

  inline RawCoordinateSystem::XDelta operator"" _dx(
      unsigned long long int value) {
    return RawCoordinateSystem::XDelta{static_cast<int>(value)};
  }
  inline RawCoordinateSystem::YDelta operator"" _dy(
      unsigned long long int value) {
    return RawCoordinateSystem::YDelta{static_cast<int>(value)};
  }
  inline RawCoordinateSystem::ZDelta operator"" _dz(
      unsigned long long int value) {
    return RawCoordinateSystem::ZDelta{static_cast<int>(value)};
  }

}

}  // namespace terra

template <typename T>
struct fmt::formatter<geometry::IntAxis<T>> {
  constexpr auto parse(format_parse_context& ctx) const {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for axis");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(const geometry::IntAxis<T>& v, FormatCtx& ctx) {
    return fmt::format_to(ctx.out(), "{}", v.ToUnderlying());
  }
};

template <typename T>
struct fmt::formatter<geometry::IntDelta<T>> {
  constexpr auto parse(format_parse_context& ctx) const {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for axis");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(const geometry::IntDelta<T>& v, FormatCtx& ctx) {
    return fmt::format_to(ctx.out(), "{}", v.ToUnderlying());
  }
};

