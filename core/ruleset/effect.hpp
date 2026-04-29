
#include <expected>
#include <core/ruleset/ruleset_base.hpp>

#include <ruleset/effect.pb.h>

namespace hs::ruleset {


    class Effect {
        Effect(const proto::ruleset::Effect& effect);

      private:
      // Just store proto object.
        proto::ruleset::Effect data_;
    };

    std::expected<void, ErrorCode> ApplyEffect(World& world, RuleSetBase& ruleset);


}
