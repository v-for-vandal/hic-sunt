#pragma once

#include <memory>

#include <core/terra/world.hpp>

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class WorldObject : public RefCounted {
  GDCLASS(WorldObject, RefCounted);

public:
  WorldObject() {}
  WorldObject(hs::terra::World&& data):
    data_(std::move(data)) {}

  using QRSCoordinateSystem = hs::terra::World::QRSCoordinateSystem;
  using QRSCoords = hs::terra::World::QRSCoords;

  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods();


private:
  hs::terra::World data_;

public:
  Vector2i get_dimensions() const;
  String get_cell_terrain(Vector2i coords) const;

  static QRSCoords to_qrs(Vector2i coords) {
    return QRSCoords{
      QRSCoordinateSystem::QAxis{coords.x},
      QRSCoordinateSystem::RAxis{coords.y}
    };
  }
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


