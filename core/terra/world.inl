#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <core/utils/serialize.hpp>
#include <stdexcept>
#include <vector>

#include "world.hpp"

namespace hs::terra {

namespace details {

enum class TopologicalVisitState { kVisiting, kVisited };

template <typename BaseTypes>
auto TopologicallySortScopePtrs(const std::vector<scope::ScopePtr<BaseTypes>>& scopes)
    -> std::vector<scope::ScopePtr<BaseTypes>> {
  using StringId = typename BaseTypes::StringId;
  absl::flat_hash_map<StringId, scope::ScopePtr<BaseTypes>> scopes_by_id;
  for (const auto& scope_ptr : scopes) {
    const auto scope_id = scope_ptr->GetId();
    if (auto [it, inserted] = scopes_by_id.try_emplace(scope_id, scope_ptr);
        !inserted && it->second.get() != scope_ptr.get()) {
      throw std::runtime_error(fmt::format("Duplicate scope {} in world", scope_id));
    }
  }

  std::vector<scope::ScopePtr<BaseTypes>> result;
  result.reserve(scopes_by_id.size());
  absl::flat_hash_map<StringId, TopologicalVisitState> states;

  auto visit = [&](this auto&& self, const scope::ScopePtr<BaseTypes>& scope_ptr) -> void {
    const auto scope_id = scope_ptr->GetId();
    if (auto state_it = states.find(scope_id); state_it != states.end()) {
      if (state_it->second == TopologicalVisitState::kVisiting) {
        throw std::runtime_error(fmt::format("Cycle detected in scope graph at {}", scope_id));
      }
      return;
    }

    states.emplace(scope_id, TopologicalVisitState::kVisiting);

    if (const auto& parent = scope_ptr->GetParent(); parent != nullptr) {
      const auto parent_id = parent->GetId();
      auto parent_it = scopes_by_id.find(parent_id);
      if (parent_it == scopes_by_id.end()) {
        throw std::runtime_error(
            fmt::format("Missing parent scope {} for scope {}", parent_id, scope_id));
      }
      self(parent_it->second);
    }

    for (const auto& tag_scope : scope_ptr->GetTagScopes()) {
      const auto tag_scope_id = tag_scope->GetId();
      auto tag_scope_it = scopes_by_id.find(tag_scope_id);
      if (tag_scope_it == scopes_by_id.end()) {
        throw std::runtime_error(
            fmt::format("Missing tag scope {} for scope {}", tag_scope_id, scope_id));
      }
      self(tag_scope_it->second);
    }

    states[scope_id] = TopologicalVisitState::kVisited;
    result.push_back(scope_ptr);
  };

  for (const auto& scope_ptr : scopes) {
    visit(scope_ptr);
  }

  return result;
}

template <typename BaseTypes>
auto TopologicallySortScopeProtos(
    const google::protobuf::RepeatedPtrField<proto::scope::Scope>& scopes)
    -> std::vector<const proto::scope::Scope*> {
  using StringId = typename BaseTypes::StringId;
  absl::flat_hash_map<StringId, const proto::scope::Scope*> scopes_by_id;
  for (const auto& scope_proto : scopes) {
    auto scope_id = ParseFrom(scope_proto.id(), serialize::To<StringId>{});
    if (auto [_, inserted] = scopes_by_id.try_emplace(scope_id, &scope_proto); !inserted) {
      throw std::runtime_error(fmt::format("Duplicate scope {} in serialized world", scope_id));
    }
  }

  std::vector<const proto::scope::Scope*> result;
  result.reserve(scopes_by_id.size());
  absl::flat_hash_map<StringId, TopologicalVisitState> states;

  auto visit = [&](this auto&& self, const proto::scope::Scope& scope_proto) -> void {
    auto scope_id = ParseFrom(scope_proto.id(), serialize::To<StringId>{});
    if (auto state_it = states.find(scope_id); state_it != states.end()) {
      if (state_it->second == TopologicalVisitState::kVisiting) {
        throw std::runtime_error(
            fmt::format("Cycle detected in serialized scope graph at {}", scope_id));
      }
      return;
    }

    states.emplace(scope_id, TopologicalVisitState::kVisiting);

    auto visit_dependency = [&](const auto& dependency_id_proto, std::string_view link_type) {
      auto dependency_id = ParseFrom(dependency_id_proto, serialize::To<StringId>{});
      if (BaseTypes::IsNullToken(dependency_id)) {
        return;
      }
      auto dependency_it = scopes_by_id.find(dependency_id);
      if (dependency_it == scopes_by_id.end()) {
        throw std::runtime_error(
            fmt::format("Missing {} scope {} for scope {}", link_type, dependency_id, scope_id));
      }
      self(*dependency_it->second);
    };

    visit_dependency(scope_proto.parent_scope_id(), "parent");
    for (const auto& tag_scope_id : scope_proto.tag_scope_ids()) {
      visit_dependency(tag_scope_id, "tag");
    }

    states[scope_id] = TopologicalVisitState::kVisited;
    result.push_back(&scope_proto);
  };

  for (const auto& scope_proto : scopes) {
    visit(scope_proto);
  }

  return result;
}

}  // namespace details

template <typename BaseTypes>
void World<BaseTypes>::VisitScopes(this auto&& self, auto&& visitor) {
  visitor(self.GetScope());

  for (auto& [_, plane] : self.planes_) {
    plane->VisitScopes(visitor);
  }

  for (auto& [_, civilization] : self.civilizations_) {
    civilization->VisitScopes(visitor);
  }
}

template <typename BaseTypes>
bool World<BaseTypes>::operator==(const World& other) const {
  if (this == &other) {
    return true;
  }

  if (planes_ != other.planes_) {
    return false;
  }

  if (civilizations_ != other.civilizations_) {
    return false;
  }

  return true;
}

template <typename BaseTypes>
auto World<BaseTypes>::GetPlane(const StringId& plane_id) const -> PlanePtr {
  if (auto fit = planes_.find(plane_id); fit != planes_.end()) {
    return fit->second;
  }

  spdlog::info("Can't find Plane with id {}", plane_id);

  return {};
}

template <typename BaseTypes>
auto World<BaseTypes>::AddPlane(const StringId& plane_id, QRSBox box, int region_radius,
                                int region_external_radius) -> PlanePtr {
  if (auto fit = planes_.find(plane_id); fit != planes_.end()) {
    spdlog::error("Plane with id {} already exists", plane_id);
    return fit->second;
  }

  spdlog::info("Making new plane with dimensions: {} and radius: {}", box, region_radius);

  StringId effective_plane_id = plane_id;
  if (BaseTypes::IsNullToken(plane_id)) {
    effective_plane_id =
        BaseTypes::StringIdFromStdString(fmt::format("plane_{}", control_object_->GetNextId()));
  }

  auto plane_ptr = std::make_shared<Plane>(control_object_, effective_plane_id, box, region_radius,
                                           region_external_radius);
  if (!plane_ptr->GetScope()->SetParent(this->GetScope())) {
    throw std::runtime_error("Can't set world as parent to plane, unrecoverable error");
  };
  planes_[effective_plane_id] = plane_ptr;

  return plane_ptr;
}

template <typename BaseTypes>
auto World<BaseTypes>::GetOrCreateCivilization(const StringId& id) -> CivilizationPtr {
  if (auto fit = civilizations_.find(id); fit != civilizations_.end()) {
    return fit->second;
  }

  CivilizationPtr civilization{id};
  if (!civilization->GetScope()->SetParent(this->GetScope())) {
    throw std::runtime_error("Can't set world as parent to civilization, unrecoverable error");
  }

  auto [it, inserted] = civilizations_.emplace(id, civilization);
  return it->second;
}

template <typename BaseTypes>
bool World<BaseTypes>::HasCivilization(const StringId& id) const noexcept {
  return civilizations_.contains(id);
}

template <typename BaseTypes>
auto World<BaseTypes>::GetCivilization(const StringId& id) const noexcept -> CivilizationPtr {
  if (auto fit = civilizations_.find(id); fit != civilizations_.end()) {
    return fit->second;
  }

  return {};
}

template <typename BaseTypes>
auto World<BaseTypes>::GetRegionById(const StringId& region_id) const noexcept -> RegionPtr {
  RegionPtr result;
  for (auto& [_, plane_ptr] : planes_) {
    result = plane_ptr->GetRegionById(region_id);
    if (result != nullptr) {
      break;
    }
  }

  return result;
}

template <typename BaseTypes>
bool World<BaseTypes>::HasRegion(const StringId& region_id) const noexcept {
  for (auto& [_, plane_ptr] : planes_) {
    if (plane_ptr->HasRegion(region_id)) {
      return true;
    }
  }

  return false;
}

template <typename BaseTypes>
void SerializeTo(const World<BaseTypes>& source, proto::terra::World& target) {
  target.Clear();
  target.set_id("world");
  target.set_scope_id(BaseTypes::ToProtoString(source.scope_->GetId()));

  std::vector<scope::ScopePtr<BaseTypes>> scopes;
  source.VisitScopes([&scopes](const auto& scope_ptr) { scopes.push_back(scope_ptr); });
  for (const auto& scope_ptr : details::TopologicallySortScopePtrs<BaseTypes>(scopes)) {
    SerializeTo(*scope_ptr, *target.add_scopes());
  }

  for (auto& [k, v] : source.planes_) {
    if (v == nullptr) {
      continue;
    }
    auto dst = target.mutable_planes()->Add();
    SerializeTo(*v, *dst);
  }

  for (auto& [_, civilization] : source.civilizations_) {
    if (civilization == nullptr) {
      continue;
    }
    auto* dst = target.add_civilizations();
    SerializeTo(*civilization, *dst);
  }

  SerializeTo(*source.control_object_, *target.mutable_control_object());
}

template <typename BaseTypes>
World<BaseTypes> ParseFrom(const proto::terra::World& source, serialize::To<World<BaseTypes>>) {
  using StringId = typename BaseTypes::StringId;
  World<BaseTypes> result;
  scope::ScopeParseContext<BaseTypes> context;

  for (const auto* scope_proto :
       details::TopologicallySortScopeProtos<BaseTypes>(source.scopes())) {
    typename World<BaseTypes>::ScopePtr scope_ptr{
        ParseFrom(*scope_proto, serialize::To<typename World<BaseTypes>::Scope>{}, context)};
    const auto& scope_id = scope_ptr->GetId();
    context.scopes_by_id.try_emplace(scope_id, scope_ptr);
  }

  const auto scope_id = ParseFrom(source.scope_id(), serialize::To<StringId>{});
  if (auto scope_it = context.scopes_by_id.find(scope_id); scope_it != context.scopes_by_id.end()) {
    result.scope_ = scope_it->second;
  } else {
    spdlog::warn("Failed to restore world scope {}", scope_id);
  }

  for (const auto& plane : source.planes()) {
    PlanePtr<BaseTypes> plane_obj = std::make_shared<Plane<BaseTypes>>(
        ParseFrom(plane, serialize::To<Plane<BaseTypes>>{}, context));
    const auto plane_id = plane_obj->GetPlaneId();
    result.planes_[plane_id] = plane_obj;
  }
  for (const auto& civilization_proto : source.civilizations()) {
    CivilizationPtr<BaseTypes> civilization_obj = std::make_shared<Civilization<BaseTypes>>(
        ParseFrom(civilization_proto, serialize::To<Civilization<BaseTypes>>{}, context));
    result.civilizations_[civilization_obj->GetId()] = civilization_obj;
  }

  if (source.has_control_object()) {
    *result.control_object_ =
        ParseFrom(source.control_object(), serialize::To<types::ControlObject>{});
  }

  result.InitNonpersistent();
  context.CheckCycles();
  return result;
}

template <typename BaseTypes>
void World<BaseTypes>::InitNonpersistent() {
  // set control object
  for (auto& [_, plane_ptr] : planes_) {
    plane_ptr->SetControlObject(control_object_);
    if (!plane_ptr->GetScope()->SetParent(this->GetScope())) {
      throw std::runtime_error("Can't set up plane parent to self");
    }
  }

  for (auto& [_, civilization_ptr] : civilizations_) {
    if (!civilization_ptr->GetScope()->SetParent(this->GetScope())) {
      throw std::runtime_error("Can't set up civilization parent to self");
    }
  }
}

}  // namespace hs::terra
