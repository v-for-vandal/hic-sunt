#pragma once

#include <fmt/format.h>

namespace hs::types {

enum class VariableType {
  kMissing,
  kNumeric,
  kString,
};

inline const char* ToString(VariableType value) noexcept {
  switch (value) {
    case VariableType::kMissing:
      return "missing";
    case VariableType::kNumeric:
      return "numeric";
    case VariableType::kString:
      return "string";
  }

  return "unknown";
}

}  // namespace hs::types

template <>
struct fmt::formatter<hs::types::VariableType> : fmt::formatter<const char*> {
  auto format(hs::types::VariableType value, format_context& ctx) const {
    return fmt::formatter<const char*>::format(hs::types::ToString(value), ctx);
  }
};
