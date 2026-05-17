#pragma once

#include "core/types/error_code.hpp"
#include "session.hpp"

#include <core/session/effect_executor.hpp>
#include <system_error>
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
      if (effect_definition->IsBroken()) {
          spdlog::warn("Skipping effect {} because it is broken. See previous logs for more details");
          continue;
      }
      try {
    auto effect_instance =
        std::make_shared<EffectInstance<BaseTypes>>(effect_definition);
    // TODO: make spdlog::debug
    spdlog::info("Successfully instantiated effect {}", effect_definition->GetId());
    effects_.push_back(std::move(effect_instance));
      } catch ( const std::system_error& e) {
          spdlog::warn("Failed to instantiate effect {} reason: {}", effect_definition->GetId(), e.what());
      }
  }

  Prepare();

  return {};
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
std::expected<void, ErrorCode> Session<BaseTypes, WorldPtr, RuleSetPtr>::SetWorld(
    WorldPtr ptr) {
    if (!ptr) {
        spdlog::error("Trying to set world to nullptr");
        return std::unexpected(ErrorCode::ERR_WORLD_MUST_BE_SET_FIRST);
    }
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

  Prepare();

  return {};
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
void Session<BaseTypes, WorldPtr, RuleSetPtr>::Prepare() {
    if(world_ == nullptr) {
        return;
    }
    if (ruleset_ == nullptr) {
        return;
    }
    // Clear cache in all scopes
    for(auto& [_, scope] : scopes_by_id_) {
        scope->ClearCache();
    }

    // TODO: Clear all modifiers that match effects not found in ruleset
    // But don't forget about manually set modifiers at world creation - those should
    // not be cleared

    // Reinitialize variable definitions at root scope
    world_->GetScope()->SetVariableDefinitions(ruleset_->GetVariableDefinitions());

    // get current turn
    auto current_turn_val = world_->GetScope()->GetNumericValue(kCoreTurn);
    if(!current_turn_val) {
        spdlog::error("Failed to retrieve {}", kCoreTurn);
        current_turn_ = 0;
    } else {
        spdlog::info("Reading current turn from world scope: {}", *current_turn_val);
        // We want to make sure that there is a modifier for this variable. Otherwise,
        // given entirely new world, we correctly read 0, because there is var definition,
        // but no modifier and no modification time
        SetCurrentTurn(*current_turn_val);
    }
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
void Session<BaseTypes, WorldPtr, RuleSetPtr>::SetCurrentTurn(size_t value) {
    current_turn_ = value;
    // Now, change core.turn variable
    auto result = world_->GetScope()->SetNumericModifier(kCoreTurn, kCoreTurn, current_turn_, 0, current_turn_);
    if (!result) {
        spdlog::error("Error when updating {}: {}", kCoreTurn, result.error());
    }
}

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
std::expected<void, ErrorCode> Session<BaseTypes, WorldPtr, RuleSetPtr>::AdvanceNextTurn() {
  if (!ruleset_) {
    spdlog::error("Ruleset is not set");
    return std::unexpected(ERR_RULESET_MUST_BE_SET_FIRST);
  }

  if (!world_) {
    spdlog::error("World is not set");
    return std::unexpected(ERR_WORLD_MUST_BE_SET_FIRST);
  }

  EffectExecutor<BaseTypes> executor;
  last_effect_execution_statistics_ = executor.Execute(*this, current_turn_);
  total_effect_execution_statistics_.MergeFrom(last_effect_execution_statistics_);

  // Now, change turn
  SetCurrentTurn(current_turn_ + 1);

  return {};
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

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
auto Session<BaseTypes, WorldPtr, RuleSetPtr>::CreateImprovementScope(StringId civ_id, StringId improvement_class) -> std::expected<ScopePtr, ErrorCode> {
    if(!world_) {
        return std::unexpected(ERR_WORLD_MUST_BE_SET_FIRST);
    }

    if(BaseTypes::IsNullToken(civ_id)) {
        spdlog::warn("Null token passed as civ_id");
        return std::unexpected(ERR_NULL_ID);
    }

    if(BaseTypes::IsNullToken(improvement_class)) {
        spdlog::warn("Null token passed as improvement_class");
        return std::unexpected(ERR_NULL_ID);
    }

    auto next_int = world_->GetNextId();
    auto scope_id = BaseTypes::StringIdFromStdString(
        fmt::format("imprv/{}", next_int));

    ScopePtr result{scope_id, types::ScopeType::SCOPE_TYPE_IMPROVEMENT};

    auto success = result->SetStringModifier(
        kCoreClass,
        kCoreClass,
        improvement_class,
        1,
        current_turn_
        );
    if (!success) {
        // This one can not happen and is unrecoverable
        throw std::runtime_error(fmt::format("Faild to set class for new scope of class {}, original error is {}",
            improvement_class,
            success.error()));
    }

    // Find civilization
    if (!world_->HasCivilization(civ_id)) {
        spdlog::warn("No such civilization: {}", civ_id);
        return std::unexpected(ERR_NO_SUCH_CIV);
    }
    auto civ = world_->GetCivilization(civ_id);

    // Find improvement class for this civilization. If it is not present,
    // create one. No-civ (empty civ_id) is handled inside this method.
    auto improvement_class_scope_id = BaseTypes::StringIdFromStdString(fmt::format("civ/{}/iclass/{}", civ_id, improvement_class));
    ScopePtr improvement_class_scope;
    if (!civ->HasChildScope(types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS, improvement_class_scope_id)) {
        // Create one
        auto creation_result = civ->CreateChildScope(types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS, improvement_class_scope_id);
        if (!creation_result) {
            utils::LogCriticalAndThrow("Failed to create scope {}; this is unrecoverable error", improvement_class_scope_id);
        }
        improvement_class_scope = *creation_result;
        // register class scope in session
        auto add_success = AddScope(improvement_class_scope);
        if (!add_success) {
            spdlog::warn("Failed to register newly created improvement class scope {}, reason: {}",
                improvement_class_scope_id, add_success.error());
            return std::unexpected(add_success.error());
        }
    } else {
        improvement_class_scope = civ->GetChildScope(types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS, improvement_class_scope_id);
    }

    // set it as a tag
    auto add_tag_link_result = result->AddTagLink(kCoreClass, improvement_class_scope);
    if(!add_tag_link_result) {
        spdlog::warn("Failed to link improvement with its class: reason {}", add_tag_link_result.error());
        return std::unexpected(add_tag_link_result.error());
    }
    auto add_scope_result = AddScope(result);
    if(!add_scope_result) {
        spdlog::warn("Failed to register newly created improvement, reasion: {}", add_scope_result.error());
        return std::unexpected(add_scope_result.error());
    }

    return result;
}

}  // namespace hs::session
