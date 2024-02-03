#pragma once

#include <memory>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/node.hpp>

#include <ui/godot/module/terra/world_object.hpp>

#include <core/system/system.hpp>


using namespace godot;

class HicSuntSystem : public Node {
  GDCLASS(HicSuntSystem, Node);

public:
  HicSuntSystem():
    system_(std::make_unique<hs::system::System>()) {}

  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods();


private:

  std::unique_ptr<hs::system::System> system_;

private:
  Ref<WorldObject> load_world(String filename);
  Ref<WorldObject> create_world(Vector2i size);
  Dictionary load_ruleset(String folder_path);
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



