#pragma once

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/string.hpp>

#include <ui/godot/module/utils/to_string.hpp>

#include <fmt/format.h>

template <>
struct fmt::formatter<::godot::String> : public fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const ::godot::String& value, FormatCtx& ctx) const {
      std::string converted = hs::godot::utils::from_string(value);
    // for some reason, constexpr doesn't work here
      return fmt::formatter<std::string>::format(converted, ctx);
  }
};

template <>
struct fmt::formatter<::godot::StringName> : public fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const ::godot::StringName& value, FormatCtx& ctx) const {
      std::string converted = hs::godot::utils::from_string(value);
    // for some reason, constexpr doesn't work here
      return fmt::formatter<std::string>::format(converted, ctx);
  }
};
