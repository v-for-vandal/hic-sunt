#pragma once

#include "core/types/error_code.hpp"
#include "session.hpp"

#include <core/session/effect_executor.hpp>
#include <spdlog/spdlog.h>

namespace hs::session {

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
std::expected<void, ErrorCode> Session<BaseTypes, WorldPtr, RuleSetPtr>::SetRuleSet(
    RuleSetPtr ruleset) {
  if (!ruleset) {
    spdlog::error("Trying to set ruleset to nullptr");
    return std::unexpected(ErrorCode::ERR_RULESET_MUST_BE_SET_FIRST);
  }

  ruleset_ = std::move(ruleset);
  effects_.clear();
  effects_.reserve(ruleset_->GetAllEffectDefinitions().size());

  for (const auto& effect_definition : ruleset_->GetAllEffectDefinitions()) {
    auto effect_instance =
        std::make_shared<EffectInstance<BaseTypes>>(effect_definition);
    effects_.push_back(std::move(effect_instance));
  }

  return {};
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
std::expected<void, ErrorCode> Session<BaseTypes, WorldPtr, RuleSetPtr>::SetWorld(
    WorldPtr ptr) {
  if (world_) {
    spdlog::error("Can not set world more than once");
    return std::unexpected(ErrorCode::ERR_WORLD_ALREADY_SET);
  }

  auto add_result = AddScope(ptr->GetScope());
  if (!add_result) {
    return add_result;
  }

  for (const auto& [_, plane] : ptr->GetPlanes()) {
    add_result = AddScope(plane->GetScope());
    if (!add_result) {
      return add_result;
    }

    plane->GetSurface().Foreach([this, &add_result](auto, auto& region_cell) {
      if (!add_result) {
        return;
      }

      auto& region = region_cell.GetRegion();

      add_result = AddScope(region.GetScope());
      if (!add_result) {
        return;
      }

      region.GetSurface().Foreach([this, &add_result](auto, auto& cell) {
        if (!add_result) {
          return;
        }
        add_result = AddScope(cell.GetScope());
      });
    });

    if (!add_result) {
      return add_result;
    }
  }

  world_ = std::move(ptr);
  return {};
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
void Session<BaseTypes, WorldPtr, RuleSetPtr>::AdvanceNextTurn() {
  if (!ruleset_) {
    spdlog::error("Ruleset is not set");
    return;
  }

  if (!world_) {
    spdlog::error("World is not set");
    return;
  }

  ++current_turn_;
  EffectExecutor<BaseTypes> executor;
  executor.Execute(*this, current_turn_);
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
std::expected<void, ErrorCode> Session<BaseTypes, WorldPtr, RuleSetPtr>::AddScope(
    const ScopePtr& scope) {
  if (scope->GetType() == ScopeType::SCOPE_TYPE_UNSPECIFIED) {
    spdlog::error("Can not add scope {} with unspecified type", scope->GetId());
    return std::unexpected(ErrorCode::ERR_SCOPE_TYPE_UNSPECIFIED);
  }

  const auto& id = scope->GetId();
  auto fit = scopes_by_id_.find(id);
  if (fit != scopes_by_id_.end()) {
    if (fit->second != scope) {
      spdlog::warn("Scope {} already exists and points to another object", id);
      return std::unexpected(ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
    }
    return {};
  }

  scopes_by_id_.emplace(id, scope);
  scopes_by_type_[scope->GetType()].push_back(scope);
  return {};
}

}  // namespace hs::session
