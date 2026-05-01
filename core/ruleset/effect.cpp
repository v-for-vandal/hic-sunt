#include "effect.hpp"
#include <expected>

namespace hs::ruleset {


# if 0
    std::expected<void, ErrorCode> ApplyEffect(Effect& effect, World& world, RuleSetBase& ruleset);

    std::expected<void, ErrorCode> ApplyWorldEffect(Session& session, size_t turn, Effect& effect, World& world, RuleSetBase& ruleset) {

       if(effect.target() != WORLD) {
           return std::unexpected(ErrorCode::ERR_INVALID_EFFECT_TARGET_TYPE);
       }

       // See if we need to run this effect
       const auto& dependencies = effect.GetDependencies();

       bool dependencies_changed = false;
       for(const auto& dependency : dependencies()) {
           // check if this variable has changed
           if (scope.GetVariableChangeToken() >= turn) {
               dependencies_changed = true;
               break;
           }
       }

       if (!dependencies_changed) {
           // No need to recalculate effect
           return;
       }

       // must recalculate effectgT

    }
#endif

}
