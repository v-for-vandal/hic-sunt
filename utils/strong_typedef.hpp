#pragma once

#include <utility>

namespace utils {

template <typename T, typename Tag>
class StrongTypedef {
 public:
  explicit constexpr StrongTypedef(T value) : value_(std::forward<T>(value)) {}

  const auto& GetUnderlying() const noexcept { return value_; }
  auto& GetUnderlying() noexcept { return value_; }

 private:
  T value_;
};

}
