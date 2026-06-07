#pragma once

#include <bitset>
#include <cstddef>

namespace hs::utils {

template <typename E, size_t N = static_cast<size_t>(E::kSize)>
class EnumBitset : public std::bitset<N> {
  static_assert(N > 0, "enum must have at least one element");

 public:
  static constexpr size_t kSize{N};
  using Base = std::bitset<N>;
  using Base::Base;

  constexpr bool operator[](E value) const { return Base::operator[](static_cast<size_t>(value)); }
  constexpr bool test(E value) const { return Base::test(static_cast<size_t>(value)); }
  constexpr bool contains(E value) const { return test(value); }

  constexpr void set() { Base::set(); }
  constexpr void reset() { Base::reset(); }
  constexpr void set(E value) { Base::set(static_cast<size_t>(value)); }
  constexpr EnumBitset& set(E value, bool state) {
    Base::set(static_cast<size_t>(value), state);
    return *this;
  }
  constexpr void reset(E value) { Base::reset(static_cast<size_t>(value)); }

  static constexpr EnumBitset Make() {
    return {};
  }

  template <typename... Values>
  static constexpr EnumBitset Make(Values... values) {
      auto result = EnumBitset{};
      (result.set(values),...);
      return result;
  }
};

template <typename E, size_t N = static_cast<size_t>(E::kSize)>
constexpr EnumBitset<E, N> MakeEnumBitset() {
  return {};
}

template <typename E, size_t N = static_cast<size_t>(E::kSize), typename... Values>
constexpr EnumBitset<E, N> MakeEnumBitset(E value, Values... values) {
  auto result = MakeEnumBitset<E, N>();
  result.set(value);
  (result.set(values), ...);
  return result;
}

}  // namespace hs::utils
