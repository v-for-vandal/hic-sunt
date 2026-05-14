#pragma once

#include <bitset>

namespace hs::utils {

template <typename E, size_t N = static_cast<size_t>(E::kSize)>
class EnumBitset : public std::bitset<N> {
  static_assert(N > 0, "enum must have at least one element");

 public:
  static constexpr size_t kSize{N};
  using Base = std::bitset<N>;
  using Base::Base;

  bool operator[](E value) const { return Base::operator[](static_cast<size_t>(value)); }
  void test(E value) { Base::test(static_cast<size_t>(value)); }
  void set() { Base::set(); }
  void reset() { Base::reset(); }
  void set(E value) { Base::set(static_cast<size_t>(value)); }
  void reset(E value) { Base::reset(static_cast<size_t>(value)); }
};

}  // namespace hs::utils
