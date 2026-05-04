#include "error_code.hpp"

namespace hs {

const char* ErrorCategory::name() const noexcept {
    return "hic-sunt";
}

std::string ErrorCategory::message(int ev) const {
    switch (static_cast<ErrorCode>(ev)) {
        case NO_ERROR:
            return "no error";
        case ERR_REPLACING_RULESET_FORBIDDEN:
            return "replacing ruleset is forbidden";
        case ERR_RULESET_MUST_BE_SET_FIRST:
            return "ruleset must be set first";
        case ERR_SCOPE_ALREADY_EXISTS:
            return "scope with this id already exists";
        case ERR_SCOPE_TYPE_UNSPECIFIED:
            return "scope type must not be unspecified";
        case ERR_WORLD_ALREADY_SET:
            return "world is already set";
        case ERR_INVALID_EFFECT_VARIABLE_REFERENCE:
            return "invalid effect variable reference";
        case ERR_EFFECT_LUA_RUNTIME_ERROR:
            return "effect lua runtime error";
        case ERR_EFFECT_LUA_OPERATION_LIMIT_EXCEEDED:
            return "effect lua operation limit exceeded";
        case ERR_INVALID_RULESET:
            return "invalid or null ruleset object";
        default:
            return "unknown hs error";
    }
}

const std::error_category& error_category() noexcept {
    static const ErrorCategory category;
    return category;
}

}
