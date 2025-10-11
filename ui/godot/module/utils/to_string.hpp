#pragma once

namespace hs::godot {

using namespace godot;

namespace utils {
inline ::godot::String to_string(const std::string& str) {
  ::godot::String result{str.c_str()};
  return result;
}
inline ::godot::StringName to_string_name(const std::string& str) {
  ::godot::StringName result{str.c_str()};
  return result;
}
inline std::string from_string(const ::godot::String& input) {
  auto utf8str = input.utf8();
  return std::string{utf8str.ptr(), static_cast<size_t>(utf8str.size())};
}
inline std::string from_string(const ::godot::StringName& str) {
  return from_string(::godot::String(str));
}
}  // namespace utils

}  // namespace hs::godot
