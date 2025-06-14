#include "pnl_object.hpp"

namespace hs::godot {

void PnlObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_total"), &PnlObject::get_total);
}


Dictionary PnlObject::get_total() const {
  Dictionary result;

  auto total = pnl_statement_.GetTotal();

  for(auto& [resource_id, value] : total) {
    result[resource_id] = value;
  }

  return result;
}

}
