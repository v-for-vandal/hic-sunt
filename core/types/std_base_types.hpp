#pragma once

#include <string>

#include <core/utils/serialize.hpp>

namespace hs {

class StdBaseTypes {
public:
  // TODO: use boost flyweight ?
  using StringId = std::string;
  using String = std::string;
  using NumericValue = double;

  static StringId StringIdFromStdString(const std::string &data) noexcept {
    return data;
  }
  static StringId StringIdFromStdString(std::string &&data) noexcept {
    return std::move(data);
  }

  static bool IsNullToken(const auto &string) noexcept {
    return string.size() == 0;
  }

  template <typename T> static auto ToProtoString(T &&input) noexcept {
    return std::forward<T>(input);
  }
  /*
  static StringId StringIdFromProtoString(const std::string& data) noexcept {
      return data;
  }
  static String StringFromProtoString(const std::string& data) noexcept {
      return data;
  }
  */
};

inline void SerializeTo(const std::string &source,
                        std::string &target) noexcept {
  target = source;
}

namespace serialize {

inline StdBaseTypes::StringId ParseFrom(const std::string &source,
                                        serialize::To<std::string>) noexcept {
  return source;
}

} // namespace serialize
} // namespace hs
