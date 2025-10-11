#include <core/types/control_object.hpp>

namespace hs::types {

void SerializeTo(const ControlObject &source,
                 proto::types::ControlObject &target) {
  target.set_next_id(source.next_id_.load());
}

ControlObject ParseFrom(const proto::types::ControlObject &source,
                        serialize::To<ControlObject>) {
  ControlObject result;
  result.next_id_.store(source.next_id());

  return result;
}

}  // namespace hs::types
