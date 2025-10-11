#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <ui/godot/module/types/godot_serialize.hpp>
#include <ui/godot/module/utils/fmt.hpp>

namespace hs::godot {

class GodotBaseTypes {
 public:
  using StringId = ::godot::StringName;

  using String = ::godot::String;

  using NumericValue = float;

  static bool IsNullToken(const auto& string) noexcept {
    return string.length() == 0;
  }

  static StringId StringIdFromStdString(const std::string& data) noexcept {
    return utils::to_string_name(data);
  }

  // This method should only be used to serialize godod Strings
  // into protobuf. For general conversion to std::string see
  // ui/godot/module/utils/to_string.hpp
  static auto ToProtoString(const ::godot::StringName& input) noexcept {
    std::string result;
    SerializeTo(input, result);
    return result;
  }
  // This method should only be used to serialize godod Strings
  // into protobuf. For general conversion to std::string see
  // ui/godot/module/utils/to_string.hpp
  static std::string ToProtoString(const ::godot::String& input) noexcept {
    std::string result;
    SerializeTo(input, result);
    return result;
  }

  /*
  static StringId StringIdFromProtoString(const std::string& data) noexcept {
      return utils::to_string_name(data);
  }
  static String StringFromProtoString(const std::string& data) noexcept {
      return utils::to_string(data);
  }
  */
};

}  // namespace hs::godot

namespace godot {

template <typename H>
H AbslHashValue(H h, const ::godot::StringName& target) {
  return H::combine(std::move(h), target.hash());
}

template <typename H>
H AbslHashValue(H h, const ::godot::String& target) {
  return H::combine(std::move(h), target.hash());
}
}  // namespace godot
