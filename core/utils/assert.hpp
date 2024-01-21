#pragma once

#include <spdlog/spdlog.h>

#include <core/utils/logging.hpp>
#include <version>

#ifndef __cpp_lib_unreachable
// a little bit of UB :)
namespace std {
[[noreturn]] inline void unreachable() {
#ifdef __GNUC__
  __builtin_unreachable();
#elif defined(_MSC_VER)
  __assume(false);
#endif
}

}  // namespace std
#endif

#define VERIFY(x)                                           \
  if (!(x)) {                                               \
    spdlog::critical("Verify failed at expression {}", #x); \
    std::terminate();                                       \
  }

#define VERIFY_MSG(x, msg, ...)                                            \
  if (!(x)) {                                                              \
    spdlog::critical("Verify failed at expression {}. Message is: {}", #x, \
                     fmt::format(msg, __VA_ARGS__));                       \
    std::terminate();                                                      \
  }

#ifdef _NDEBUG
#define DEBUG_VERIFY(x)
#define DEBUG_VERIFY_MSG(x, msg, ...)
#else
#define DEBUG_VERIFY(x) VERIFY(x)
#define DEBUG_VERIFY_MSG(x, msg, ...) VERIFY_MSG(x, msg, __VA_ARGS__)
#endif

#define CHECK(x) DEBUG_VERIFY(x)
