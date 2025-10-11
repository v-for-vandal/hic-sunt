#pragma once

namespace hs::utils {

template <typename... T>
struct Overloaded : public T... {
  using T::operator()...;
};

}  // namespace hs::utils
