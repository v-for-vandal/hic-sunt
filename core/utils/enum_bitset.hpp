#pragma once

#include <bitset>
#include <utility>

namespace hs::utils {

template <typename E>
class EnumBitset : public std::bitset<static_cast<size_t>(E::kSize)> {
  // TODO: use std::to_underlying as soon as it is ready
  static_assert(static_cast<int64_t>(E::kSize) > 0,
                "enum must have at least one element");

public:
  static constexpr int kSize{E::kSize};
  using Base = std::bitset<static_cast<size_t>(E::kSize)>;
  using Base::Base;

  bool operator[](E value) const {
    return Base::operator[](static_cast<size_t>(value));
  }
  void test(E value) { Base::test(static_cast<size_t>(value)); }
  void set() { Base::set(); }
  void reset() { Base::reset(); }
  void set(E value) { Base::set(static_cast<size_t>(value)); }
  void reset(E value) { Base::reset(static_cast<size_t>(value)); }
};

} // namespace hs::utils
