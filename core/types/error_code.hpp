#pragma once

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

namespace std
{
    template<>
    struct is_error_code_enum<hs::ErrorCode>:
        true_type
    {};
}
