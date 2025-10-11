#pragma once

#include <memory>

namespace hs::utils {

// Shared ptr that can never be null. Class T must be default-constructible.
template <typename T>
class NonNullSharedPtr {
 private:
  std::shared_ptr<T> value;

 public:
  template <typename... Args>
  NonNullSharedPtr(Args... args)
      : value(std::make_shared<T>(std::forward<Args>(args)...)) {}

  NonNullSharedPtr(NonNullSharedPtr<T> &&other) {
    value = std::move(other.value);
    other.reset();
  }
  NonNullSharedPtr(const NonNullSharedPtr<T> &other) = default;
  NonNullSharedPtr<T> &operator=(NonNullSharedPtr<T> &&other) {
    value = std::move(other.value);
    other.reset();

    return *this;
  }
  NonNullSharedPtr<T> &operator=(const NonNullSharedPtr<T> &other) = default;
  NonNullSharedPtr<T> &operator=(const std::shared_ptr<T> &other) {
    if (other == nullptr) {
      reset();
    } else {
      value = other;
    }
  }

  operator std::shared_ptr<T>() const { return value; }

  operator const std::shared_ptr<T> &() { return value; }

  operator bool() const { return bool(value); }

  auto &operator*() { return *value; }
  const auto &operator*() const { return *value; }
  auto &operator->() { return value; }
  const auto &operator->() const { return value; }

  bool operator==(const NonNullSharedPtr<T> &other) const noexcept = default;
  bool operator==(const std::shared_ptr<T> &other) const noexcept {
    return value == other;
  }

  bool operator!=(const NonNullSharedPtr<T> &other) const noexcept = default;

  bool operator==(nullptr_t) const noexcept { return value == nullptr; }
  bool operator!=(nullptr_t) const noexcept { return value != nullptr; }

  void reset() { value = std::make_shared<T>(); }
};

}  // namespace hs::utils
