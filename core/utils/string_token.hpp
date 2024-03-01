#pragma once

#include <string>

namespace hs::utils {

using StringToken = std::string;
using StringTokenCRef = const std::string&;

inline bool IsNullToken(StringTokenCRef token) {
  return token.empty();
}

}

namespace hs {
  using hs::utils::StringToken;
  using hs::utils::StringTokenCRef;
}
