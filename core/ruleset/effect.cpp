#include "effect.hpp"

namespace hs::ruleset {


    std::expected<void, ErrorCode> ApplyEffect(Effect& effect, World& world, RuleSetBase& ruleset);
}
