#include "cell_object.hpp"

#include <ui/godot/module/utils/cast_qrs.hpp>

#include <godot_cpp/core/object.hpp>

#define ERR_FAIL_REGION_NO_CELL(result) ERR_FAIL_COND_V_MSG(\
  (!region_->GetSurface().Contains(cell_coords_)),\
  (result),\
  ERR_MSG_REGION_NO_CELL)

#define ERR_FAIL_NULL_REGION(result) ERR_FAIL_NULL_V_MSG(region_, result, ERR_MSG_REGION_IS_NULL)


namespace hs::godot {

static constexpr const char* ERR_MSG_REGION_NO_CELL = "No such cell in region";
static constexpr const char* ERR_MSG_REGION_IS_NULL = "null-containing region object";

void CellObject::_bind_methods() {
  ScopeMixin::_bind_methods<CellObject>();
  //ClassDB::bind_method(D_METHOD("your_method"), &CellObject::your_method);
}

ScopePtr CellObject::GetScope() const
{
  ERR_FAIL_NULL_REGION(Ref<ScopeObject>{});
  ERR_FAIL_REGION_NO_CELL(Ref<ScopeObject>{});

  auto& cell = region_->GetSurface().GetCell(cell_coords_);

  return cell.GetScope();
}


String CellObject::get_region_id() const {
  String result;
  ERR_FAIL_NULL_V_MSG(region_, result, "Failed to get scope object");
  if(!region_) {
    return {};
  }

  return region_->GetId();

}

}

