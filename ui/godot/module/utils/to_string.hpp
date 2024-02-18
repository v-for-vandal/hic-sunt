#pragma once

using namespace godot;

namespace utils {
  inline String to_string(const std::string& str) {
    String result{str.c_str()};
    return result;
  }
}
