#pragma once

#include <vector>

#include "google/protobuf/repeated_field.h"

namespace hs::serialize {

  template<typename T>
  struct To {
    using Target = T;
  };

}

namespace hs {

  template<typename T, typename ProtoT>
  void SerializeTo(const std::vector<T>& source,
    ::google::protobuf::RepeatedPtrField<ProtoT>& target) {
    target.Clear();
    target.Reserve(source.size());
    for(const auto& elem : source) {
      auto target_elem_ptr = target.Add();
      SerializeTo(elem, *target_elem_ptr);
    }
  }

}
