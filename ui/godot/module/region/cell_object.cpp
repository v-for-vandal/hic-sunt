#include "cell_object.hpp"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>
#include <ui/godot/module/utils/cast_qrs.hpp>

#define ERR_FAIL_REGION_NO_CELL(result)                                          \
  ERR_FAIL_COND_V_MSG((!region_->GetSurface().Contains(cell_coords_)), (result), \
                      ERR_MSG_REGION_NO_CELL)

#define ERR_FAIL_NULL_REGION(result) ERR_FAIL_NULL_V_MSG(region_, result, ERR_MSG_REGION_IS_NULL)

namespace hs::godot {

static constexpr const char* ERR_MSG_REGION_NO_CELL = "No such cell in region";
static constexpr const char* ERR_MSG_REGION_IS_NULL = "null-containing region object";

static constexpr const char* INVALID_CELL_ID = "invalid-cell";

void CellObject::_bind_methods() {
  ScopeMixin::_bind_methods<CellObject>();
  ClassDB::bind_method(D_METHOD("is_valid"), &CellObject::is_valid);
  ClassDB::bind_method(D_METHOD("get_improvement", "slot"), &CellObject::get_improvement);
  ClassDB::bind_method(D_METHOD("get_improvements"), &CellObject::get_improvements);
  ClassDB::bind_method(D_METHOD("get_id"), &CellObject::get_id);
  ClassDB::bind_method(D_METHOD("get_region_id"), &CellObject::get_region_id);
}

ScopePtr CellObject::CreateInvalidCellScope() {
  return ScopePtr{INVALID_CELL_ID, types::ScopeType::SCOPE_TYPE_CELL};
}

ScopePtr CellObject::GetScope() const {
  ERR_FAIL_NULL_REGION(CreateInvalidCellScope());
  ERR_FAIL_REGION_NO_CELL(CreateInvalidCellScope());

  auto& cell = region_->GetSurface().GetCell(cell_coords_);

  return cell.GetScope();
}

auto CellObject::GetCell() -> Cell& {
  if (!region_) {
    static Cell fallback;
    ERR_FAIL_NULL_REGION(fallback);
  }

  return region_->GetSurface().GetCell(cell_coords_);
}

Ref<ScopeObject> CellObject::get_improvement(int slot) {
  ERR_FAIL_NULL_REGION(Ref<ScopeObject>{});
  ERR_FAIL_REGION_NO_CELL(Ref<ScopeObject>{});

  auto& cell = region_->GetSurface().GetCell(cell_coords_);

  auto improvement_scope = cell.GetImprovement(slot);
  if (!improvement_scope || improvement_scope->IsOrphaned()) {
    return Ref<ScopeObject>{};
  }

  Ref<ScopeObject> result(memnew(ScopeObject(improvement_scope)));
  ERR_FAIL_NULL_V_MSG(result.ptr(), Ref<ScopeObject>{}, "failed to create scope object");
  return result;
}

auto CellObject::get_improvements() -> TypedDictionary<int, ScopeObject> {
  TypedDictionary<int, ScopeObject> result;
  ERR_FAIL_NULL_REGION(result);
  ERR_FAIL_REGION_NO_CELL(result);

  const auto& improvements = region_->GetSurface().GetCell(cell_coords_).GetImprovements();
  for (const auto& [slot, improvement_scope] : improvements) {
    if (!improvement_scope || improvement_scope->IsOrphaned()) {
      continue;
    }

    Ref<ScopeObject> improvement(memnew(ScopeObject(improvement_scope)));
    ERR_FAIL_NULL_V_MSG(improvement.ptr(), result, "failed to create scope object");
    result[slot] = improvement;
  }

  return result;
}

StringName CellObject::get_region_id() const {
  ERR_FAIL_NULL_REGION(StringName{});

  return region_->GetId();
}

}  // namespace hs::godot
