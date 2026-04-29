#include "session.hpp"
#include "spdlog/spdlog.h"

namespace hs::system {

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
auto Session<BaseTypes, WorldPtr, RuleSetPtr>::SetRuleSet(RuleSetPtr ptr) -> std::expected<void, StringId> {

    // Replacing RuleSet is forbidden

    if (ruleset_) {
        return std::unexpected(StringId{"ERR_REPLACING_RULESET_FORBIDDEN"});
    }

    ruleset_ = ptr;
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
auto Session<BaseTypes,WorldPtr, RuleSetPtr>::SetWorld(WorldPtr ptr) -> std::expected<void, StringId> {

    // Ruleset must be set first

    if (!ruleset_) {
        return std::unexpected(StringId{"ERR_RULESET_MUST_BE_SET_FIRST"});
    }

    // TODO: Clean up all modifiers in all scopes that do not correspond to
    // our current ruleset

    world_ = ptr;
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
void Session<BaseTypes, WorldPtr, RuleSetPtr>::AdvanceNextTurn() {
    if(!ruleset_) {
        SPDLOG_ERROR("Ruleset is not set!");
        return;
    }
    if(!world_) {
        SPDLOG_ERROR("World is not set!");
        return;
    }
    // Get all effects and run them
    for (const auto& effect : ruleset_->GetAllEffects()) {
        // apply effect
    }


}
}
