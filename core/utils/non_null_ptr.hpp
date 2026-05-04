#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace hs::utils {

// Shared ptr that can never be null. Class T must be default-constructible.
template <typename T>
class NonNullSharedPtr {
 private:
  template <typename U>
  static constexpr bool kCompatibleSharedPtr =
      std::is_convertible_v<U*, T*>;

  std::shared_ptr<T> value;

 public:
  NonNullSharedPtr() : value(std::make_shared<T>()) {}

  template <typename... Args>
    requires(std::constructible_from<T, Args...>)
  explicit NonNullSharedPtr(Args&&... args)
      : value(std::make_shared<T>(std::forward<Args>(args)...)) {}

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr(const std::shared_ptr<U>& other) {
    *this = other;
  }

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr(std::shared_ptr<U>&& other) {
    *this = std::move(other);
  }

  NonNullSharedPtr(NonNullSharedPtr<T>&& other) {
    value = std::move(other.value);
    other.reset();
  }
  NonNullSharedPtr(const NonNullSharedPtr<T>& other) = default;

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr(const NonNullSharedPtr<U>& other) {
    value = static_cast<std::shared_ptr<T>>(other);
  }

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr(NonNullSharedPtr<U>&& other) {
    value = static_cast<std::shared_ptr<T>>(std::move(other));
  }

  NonNullSharedPtr<T>& operator=(NonNullSharedPtr<T>&& other) {
    value = std::move(other.value);
    other.reset();

    return *this;
  }

  NonNullSharedPtr<T>& operator=(const NonNullSharedPtr<T>& other) = default;

  NonNullSharedPtr<T>& operator=(T&& data) {
      value = std::make_shared<T>(std::move(data));
      return * this;
  }

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr<T>& operator=(const std::shared_ptr<U>& other) {
    if (other == nullptr) {
      reset();
    } else {
      value = other;
    }
    return *this;
  }

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr<T>& operator=(std::shared_ptr<U>&& other) {
    if (other == nullptr) {
      reset();
    } else {
      value = std::move(other);
    }
    return *this;
  }

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr<T>& operator=(const NonNullSharedPtr<U>& other) {
    value = static_cast<std::shared_ptr<T>>(other);
    return *this;
  }

  template <typename U>
    requires(kCompatibleSharedPtr<U>)
  NonNullSharedPtr<T>& operator=(NonNullSharedPtr<U>&& other) {
    value = static_cast<std::shared_ptr<T>>(std::move(other));
    return *this;
  }

  operator std::shared_ptr<T>() const { return value; }

  operator const std::shared_ptr<T>&() const { return value; }

  operator bool() const { return bool(value); }

  auto& operator*() { return *value; }
  const auto& operator*() const { return *value; }
  auto& operator->() { return value; }
  const auto& operator->() const { return value; }

  bool operator==(const NonNullSharedPtr<T>& other) const noexcept = default;
  bool operator==(const std::shared_ptr<T>& other) const noexcept {
    return value == other;
  }

  bool operator!=(const NonNullSharedPtr<T>& other) const noexcept = default;

  bool operator==(nullptr_t) const noexcept { return value == nullptr; }
  bool operator!=(nullptr_t) const noexcept { return value != nullptr; }

  bool IsEmpty() const noexcept { return value == nullptr; }

  void reset() { value = std::make_shared<T>(); }
};

}  // namespace hs::utils
