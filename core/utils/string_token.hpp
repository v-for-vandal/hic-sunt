#pragma once

#include <string>

/* TODO: Use boost flyweight library to do string interning */

/*
namespace hs::utils {

using StringToken = std::string;
using StringTokenCRef = const std::string&;

inline bool IsNullToken(StringTokenCRef token) {
  return token.empty();
}

inline const std::string& ToString(StringTokenCRef token) {
  return token;
}

}

namespace hs {
  using hs::utils::StringToken;
  using hs::utils::StringTokenCRef;
}
*/
