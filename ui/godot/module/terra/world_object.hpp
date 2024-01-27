#pragma once

#include <memory>

#include <core/terra/world.hpp>

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class WorldObject : public RefCounted {
  GODOT_CLASS(WorldObject, RefCounted);

public:
  WorldObject() {}

  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _register_methods();


private:
  /*
  Vector3 GetDimensions() const { return end_point_ - start_point_; }
  Vector3 GetStartPoint() const { return start_point_; }
  Vector3 GetEndPoint() const { return end_point_; }
  Vector3 GetMcStartPoint() const { return mc_start_point_; }
  Vector3 GetMcEndPoint() const { return mc_end_point_; }
  Ref<CellObject> GetCellObject(Vector3 coords) const;
  bool IsMainLevel(int level) const;

  Vector3 start_point_{0,0,0};
  Vector3 end_point_{0,0,0};
  Vector3 mc_start_point_{0,0,0};
  Vector3 mc_end_point_{0,0,0};
  std::shared_ptr<terra::World> world_ptr_;
  */

};


