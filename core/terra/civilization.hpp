#pragma once

#include <absl/container/flat_hash_map.h>
#include <terra/civilization.pb.h>

#include <core/scope/scope.hpp>
#include <core/scope/scoped_object.hpp>
#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>
#include <expected>

namespace hs::terra {

template <typename BaseTypes>
class Civilization;

template <typename BaseTypes>
void SerializeTo(const Civilization<BaseTypes>& source, proto::terra::Civilization& target);

template <typename BaseTypes>
Civilization<BaseTypes> ParseFrom(const proto::terra::Civilization& source,
                                  serialize::To<Civilization<BaseTypes>>,
                                  const scope::ScopeParseContext<BaseTypes>& context);

/* \brief Class that incapsulates working with scope of type SCOPE_TYPE_CIV
 *
 * This class is not intended to store all complexity of civilization - this logic lives in Godot.
 * Instead, we have here methods that simplify working with scopes.
 */
template <typename BaseTypes = StdBaseTypes>
class Civilization : public scope::TypedScopedObject<BaseTypes, types::ScopeType::SCOPE_TYPE_CIV> {
 public:
  using Base = scope::TypedScopedObject<BaseTypes, types::ScopeType::SCOPE_TYPE_CIV>;
  using Scope = scope::Scope<BaseTypes>;
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using ScopeType = types::ScopeType;
  using StringId = typename BaseTypes::StringId;
  using ScopeMap = absl::flat_hash_map<StringId, ScopePtr>;
  using ScopedChildrenMap = absl::flat_hash_map<ScopeType, ScopeMap>;

  Civilization() = default;
  explicit Civilization(StringId id);

  /* \brief Return civilization id */
  auto GetId() const noexcept { return Base::GetScope()->GetId(); }

  // Add given scope. Scope must be new, as it will be added as our child.
  [[nodiscard]] std::expected<void, ErrorCode> AddChildScope(const ScopePtr& scope);
  // Creates new scope and adds it. If scope with this id exists (within this class), then
  // error is returned
  [[nodiscard]] std::expected<ScopePtr, ErrorCode> CreateChildScope(ScopeType scope_type,
                                                                    const StringId& id);
  bool HasChildScope(ScopeType scope_type, const StringId& id) const;

  [[nodiscard]] std::expected<ScopePtr, ErrorCode> GetOrCreateChildScope(ScopeType scope_type,
                                                                         const StringId& id);
  // This method will return default-initialized ScopePtr if no such id is present. Remember that
  // ScopePtr is non-nullable and will always contain data. It is very hard to check ScopePtr for
  // 'validness' - it is better to check HasScope before calling this method.
  ScopePtr GetChildScope(ScopeType scope_type, const StringId& id) const;

  const ScopedChildrenMap& GetChildScopesByType() const noexcept;
  const ScopeMap& GetChildScopes() const noexcept;
  const ScopeMap* FindScopesByType(ScopeType scope_type) const noexcept;

  void VisitScopes(this auto&& self, auto&& visitor);

  /* \brief Creates class scope for given improvement class id.
   *
   * There is no check that this improvement_class id is present in the system. This method
   * simply initializes scope properly and adds it with AddChildScope. If such scope already exists,
   * ERR_SCOPE_ALREADY_EXISTS is returned

   Note: bad idea, has no access to modification time and ruleset
  std::expected<ScopePtr, ErrorCode> CreateImprovementClassScope(StringId civ_id, StringId
  improvement_class);
  */

 private:
  friend void SerializeTo<BaseTypes>(const Civilization& source,
                                     proto::terra::Civilization& target);
  friend Civilization ParseFrom<BaseTypes>(const proto::terra::Civilization& source,
                                           serialize::To<Civilization>,
                                           const scope::ScopeParseContext<BaseTypes>& context);

  ScopedChildrenMap child_scopes_;
  ScopeMap all_child_scopes_;

  StringId kCoreClass{"core.class"};
};

}  // namespace hs::terra

#include "civilization.inl"
