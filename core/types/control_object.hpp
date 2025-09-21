#pragma once

#include <core/utils/non_null_ptr.hpp>
#include <core/utils/serialize.hpp>

#include <types/control_object.pb.h>

namespace hs::types {

class ControlObject;
using ControlObjectPtr = ::hs::utils::NonNullSharedPtr<ControlObject>;

void SerializeTo(const ControlObject &source,
                 proto::types::ControlObject &target);
ControlObject ParseFrom(const proto::types::ControlObject &source,
                        serialize::To<ControlObject>);

class ControlObject {
public:
  auto GetNextId() const noexcept { return next_id_.fetch_add(1); }

  ControlObject() = default;
  ControlObject(const ControlObject &other) : next_id_(other.next_id_.load()) {}
  ControlObject &operator=(const ControlObject &other) {
    next_id_.store(other.next_id_.load());
    return *this;
  }

  static ControlObjectPtr Create() { return ControlObjectPtr{}; }

private:
  friend void SerializeTo(const ControlObject &source,
                          proto::types::ControlObject &target);
  friend ControlObject ParseFrom(const proto::types::ControlObject &world,
                                 serialize::To<ControlObject>);

  mutable std::atomic<int64_t> next_id_{0};
};

} // namespace hs::types

namespace hs {
using ControlObjectPtr = ::hs::types::ControlObjectPtr;
}
