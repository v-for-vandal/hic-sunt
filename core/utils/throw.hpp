#pragma once

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <stdexcept>
#include <string_view>

namespace hs::utils {

template <typename... Args>
[[noreturn]] void LogCriticalAndThrow(std::string_view format, Args&&... args) {
  auto message = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
  spdlog::critical(message);
  throw std::runtime_error(message);
}

}  // namespace hs::utils
