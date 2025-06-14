#pragma once

#include <core/types/std_base_types.hpp>

#include "google/protobuf/repeated_field.h"

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

  template<typename M, typename ProtoM>
  void SerializeMapTo(const M& source, ProtoM* target) {
      target->clear();
      target->insert(source.begin(),
          source.end());
  }

  template<typename M, typename ProtoM>
  void ParseMapFrom(const ProtoM& source, M& target) {
      target.clear();
      target.insert(source.begin(),
          source.end());
  }

  template<typename BaseTypes, typename M, typename ProtoM>
  void SerializeStringIdMapTo(BaseTypes, const M& source, ProtoM* target) {
      target->clear();
      using String = typename BaseTypes::String;
      for(const auto & [k,v] : source) {
          std::string key;
          SerializeTo(k, key);
          if constexpr (std::is_same_v<typename M::mapped_type, String>) {
              //target->insert(MapPair{bt.ToProtoString(k), bt.ToProtoString(v)});
              SerializeTo(v, (*target)[key]);
          } else {
              (*target)[key] = v;
              //target->insert(MapPair{bt.ToProtoString(k), v});
          }
      }
  }

  template<typename BaseTypes, typename M, typename ProtoM>
  void ParseStringIdMapFrom(BaseTypes, const ProtoM& source, M& target) {
      target.clear();
      using String = typename BaseTypes::String;
      using StringId = typename BaseTypes::StringId;
      for(const auto & [k,v] : source) {
          StringId key = ParseFrom(k, serialize::To<StringId>{});

          if constexpr (std::is_same_v<typename M::mapped_type, String>) {
              target[key] = ParseFrom(v, serialize::To<typename M::mapped_type>{});
          } else {
              target[key] = v;
          }
      }
  }

}
