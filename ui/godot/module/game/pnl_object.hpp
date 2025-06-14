#pragma once

#include <memory>

#include <ui/godot/module/game/pnl_statement.hpp>

#include <godot_cpp/classes/ref.hpp>


namespace hs::godot {

using namespace ::godot;

class PnlObject : public RefCounted {
  GDCLASS(PnlObject, RefCounted);

public:
  PnlObject() {}
  PnlObject(PnlStatement&& data):
    pnl_statement_(std::move(data)) {}


  void _init() {
  }

  static void _bind_methods();

  void SetPnlStatement(PnlStatement statement) {
    pnl_statement_ = std::move(statement);
  }


private:
  PnlStatement pnl_statement_;

public:
  Dictionary get_total() const;
};

}
