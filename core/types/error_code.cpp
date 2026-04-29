#include "error_code.hpp"

namespace hs {

std::string ErrorCategory::message(int ev) const {
    switch (static_cast<ErrorCode>(ev)) {
        case NO_ERROR:
            return "no error";
        case ERR_REPLACING_RULESET_FORBIDDEN:
            return "replacing ruleset is forbidden";
        case ERR_RULESET_MUST_BE_SET_FIRST:
            return "ruleset must be set first";
        default:
            return "unknown hs error";
    }
}

}
