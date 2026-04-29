#pragma once

#include <core/ruleset/ruleset.hpp>
#include <core/terra/world.hpp>
#include <filesystem>
#include <memory>
#include <expected>

namespace hs::system {

template <typename BaseTypes = StdBaseTypes, typename WorldPtr, typename RuleSetPtr>
class Session {
public:
    using StringId = typename BaseTypes::StringId;
    Session();

    std::expected<void, ErrorCode> SetRuleSet(RuleSetPtr ptr);

    std::expected<void, ErrorCode> SetWorld(WorldPtr ptr);

    // Advance next turn will move timeline to next turn and execute all
    // required scripts, functions and so on.
    void AdvanceNextTurn();

    // This function will change internal turn counter without any logic. Its
    // primary use is to set current turn when loading game. Changing turn
    // to earlier value is forbidden because it messes up caches.
    void SetCurrentTurn(size_t value) { current_turn_ = value; }

    // Return current turn
    size_t GetCurrentTurn() const { return current_turn_; }

private:
    RuleSetPtr ruleset_;
    WorldPtr world_;
    std::size_t current_turn_{0};

};

}  // namespace hs::system

#include "session.inl"
