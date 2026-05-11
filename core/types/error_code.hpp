#pragma once

#include <fmt/format.h>

#include <system_error>

namespace hs {

    enum ErrorCode {
        NO_ERROR = 0,
        // Session errors
        ERR_REPLACING_RULESET_FORBIDDEN,
        ERR_RULESET_MUST_BE_SET_FIRST,

        // variables errors
        ERR_EMPTY_MODIFIER_KEY,
        ERR_NO_SUCH_VARIABLE,
        ERR_INCORRECT_VARIABLE_TYPE,

        // session errors
        ERR_SCOPE_ALREADY_EXISTS,
        ERR_SCOPE_TYPE_UNSPECIFIED,
        ERR_WORLD_ALREADY_SET,
        ERR_INVALID_RULESET,

        // effect errors
        ERR_INVALID_EFFECT_VARIABLE_REFERENCE,
        ERR_EFFECT_LUA_RUNTIME_ERROR,
        ERR_EFFECT_LUA_OPERATION_LIMIT_EXCEEDED
    };

    class ErrorCategory final : public std::error_category {
     public:
        const char* name() const noexcept override;

        std::string message(int ev) const override;

        ~ErrorCategory() noexcept override {}
    };

    const std::error_category& error_category() noexcept;

    inline std::error_code make_error_code(ErrorCode error) noexcept {
        return {static_cast<int>(error), error_category()};
    }
}

template <>
struct fmt::formatter<hs::ErrorCode> : fmt::formatter<std::string_view> {
    auto format(hs::ErrorCode error, format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(
            hs::error_category().message(static_cast<int>(error)), ctx);
    }
};

namespace std
{
    template<>
    struct is_error_code_enum<hs::ErrorCode>:
        true_type
    {};
}
