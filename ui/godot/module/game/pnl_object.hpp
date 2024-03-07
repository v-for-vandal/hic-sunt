#pragma once

#include <memory>

#include <core/region/pnl_statement.hpp>

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class PnlObject : public RefCounted {
  GDCLASS(PnlObject, RefCounted);

public:
  PnlObject() {}
  PnlObject(hs::region::PnlStatement&& data):
    pnl_statement_(std::move(data)) {}


  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods();

  void SetPnlStatement(hs::region::PnlStatement statement) {
    pnl_statement_ = std::move(statement);
  }


private:
  hs::region::PnlStatement pnl_statement_;

public:
  Dictionary get_total() const;
};


