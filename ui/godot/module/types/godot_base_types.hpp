#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <sol/forward.hpp>
#include <sol/sol.hpp>
#include <ui/godot/module/types/godot_serialize.hpp>
#include <ui/godot/module/utils/fmt.hpp>
#include <ui/godot/module/utils/to_string.hpp>

#include <string>

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

template <typename Handler>
bool sol_lua_check(sol::types<::godot::StringName>, lua_State* L, int index,
                   Handler&& handler, sol::stack::record& tracking) {
  const int absolute_index = lua_absindex(L, index);
  const bool success =
      sol::stack::check<std::string>(L, absolute_index,
                                     std::forward<Handler>(handler), tracking);
  //tracking.use(1);
  return success;
}

inline ::godot::StringName sol_lua_get(sol::types<::godot::StringName>, lua_State* L,
                                int index, sol::stack::record& tracking) {
  const int absolute_index = lua_absindex(L, index);
  const std::string value =
      sol::stack::get<std::string>(L, absolute_index, tracking);
  //tracking.use(1);
  return hs::godot::utils::to_string_name(value);
}

inline int sol_lua_push(sol::types<::godot::StringName>, lua_State* L,
                 const ::godot::StringName& value) {
  return sol::stack::push(L, hs::godot::utils::from_string(value));
}

template <typename Handler>
bool sol_lua_check(sol::types<::godot::String>, lua_State* L, int index,
                   Handler&& handler, sol::stack::record& tracking) {
  const int absolute_index = lua_absindex(L, index);
  const bool success =
      sol::stack::check<std::string>(L, absolute_index,
                                     std::forward<Handler>(handler), tracking);
  //tracking.use(1);
  return success;
}

inline ::godot::String sol_lua_get(sol::types<::godot::String>, lua_State* L,
                            int index, sol::stack::record& tracking) {
  const int absolute_index = lua_absindex(L, index);
  const std::string value =
      sol::stack::get<std::string>(L, absolute_index, tracking);
  //tracking.use(1);
  return hs::godot::utils::to_string(value);
}

inline int sol_lua_push(sol::types<::godot::String>, lua_State* L,
                 const ::godot::String& value) {
  return sol::stack::push(L, hs::godot::utils::from_string(value));
}
