#pragma once

#include <memory>

namespace hs::utils {

// The sole purpose is to provide operator== that compares items
// by value, not by address
template<typename T>
class SharedPtr {
public:
  std::shared_ptr<T> value;

  template<typename ... Args>
  SharedPtr(Args ... args):
    value(std::forward<Args>(args)...)
  {}

  SharedPtr(SharedPtr<T>&& other) = default;
  SharedPtr(const SharedPtr<T>& other) = default;
  SharedPtr<T>& operator=(SharedPtr<T>&& other) = default;
  SharedPtr<T>& operator=(const SharedPtr<T>& other) = default;

  operator std::shared_ptr<T>() const {
    return value;
  }

  operator const std::shared_ptr<T>& () {
    return value;
  }

  operator bool () const {
      return bool(value);
  }

  auto& operator*() { return *value; }
  const auto& operator*() const { return *value; }
  auto& operator->() { return value; }
  const auto& operator->() const { return value; }

  bool operator==(const SharedPtr<T>& other) const noexcept {
    if (value == other.value) {
      return true;
    }

    if (value != nullptr && other.value != nullptr) {
      return *value == *other.value;
    }

    return false;
  }

  bool operator !=(const SharedPtr<T>& other) const noexcept {
    return !(*this == other);
  }
  bool operator==(nullptr_t) const noexcept {
    return value == nullptr;
  }
  bool operator!=(nullptr_t) const noexcept {
    return value != nullptr;
  }

};

}
