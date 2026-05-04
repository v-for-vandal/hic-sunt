#pragma once

#include <godot_cpp/classes/ref.hpp>

#include <core/session/session.hpp>
#include <ui/godot/module/game/ruleset.hpp>
#include <ui/godot/module/game/ruleset_object.hpp>
#include <ui/godot/module/terra/world.hpp>
#include <ui/godot/module/terra/world_object.hpp>
#include <ui/godot/module/types/godot_base_types.hpp>

namespace hs::godot {

template <typename ObjectType, typename DataType>
class RefDataPtrAdapter {
 public:
  RefDataPtrAdapter() = default;
  explicit RefDataPtrAdapter(const godot::Ref<ObjectType>& ref) : ref_(ref) {}

  DataType& operator*() { return const_cast<DataType&>(GetData()); }
  const DataType& operator*() const { return GetData(); }

  DataType* operator->() { return std::addressof(const_cast<DataType&>(GetData())); }
  const DataType* operator->() const { return std::addressof(GetData()); }

  explicit operator bool() const noexcept { return ref_.ptr() != nullptr; }

  friend bool operator==(const RefDataPtrAdapter& adapter,
                         std::nullptr_t) noexcept {
    return !static_cast<bool>(adapter);
  }

  friend bool operator==(std::nullptr_t,
                         const RefDataPtrAdapter& adapter) noexcept {
    return !static_cast<bool>(adapter);
  }

  friend bool operator!=(const RefDataPtrAdapter& adapter,
                         std::nullptr_t) noexcept {
    return static_cast<bool>(adapter);
  }

  friend bool operator!=(std::nullptr_t,
                         const RefDataPtrAdapter& adapter) noexcept {
    return static_cast<bool>(adapter);
  }

 private:
  const DataType& GetData() const;

  godot::Ref<ObjectType> ref_;
};

template <>
inline const RuleSet& RefDataPtrAdapter<RulesetObject, RuleSet>::GetData() const {
  return ref_->GetRuleSet();
}

template <>
inline const World& RefDataPtrAdapter<WorldObject, World>::GetData() const {
  return ref_->GetWorld();
}

using RuleSetPtrAdapter = RefDataPtrAdapter<RulesetObject, RuleSet>;
using WorldPtrAdapter = RefDataPtrAdapter<WorldObject, World>;
using Session = hs::session::Session<GodotBaseTypes, WorldPtrAdapter, RuleSetPtrAdapter>;

}  // namespace hs::godot
