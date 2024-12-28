#pragma once

#include <memory>

#include <core/terra/world.hpp>
#include <ui/godot/module/game/pnl_object.hpp>
#include <ui/godot/module/game/ruleset_object.hpp>

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class RegionObject : public RefCounted {
  GDCLASS(RegionObject, RefCounted);

public:
  RegionObject() {}
  RegionObject(std::shared_ptr<hs::region::Region> region):
    region_(std::move(region)) {}

  using QRSCoordinateSystem = hs::terra::World::QRSCoordinateSystem;
  using QRSCoords = hs::terra::World::QRSCoords;

  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods();

  static Dictionary make_region_info(const hs::region::Region& region);


private:
  std::shared_ptr<hs::region::Region> region_; // never null

public:
  Rect2i get_dimensions() const;
  Dictionary get_cell_info(Vector2i coords) const;
  bool set_biome(Vector2i coords, String biome) const;
  bool set_feature(Vector2i coords, String feature) const;
  bool set_improvement(Vector2i coords, String improvement) const;
  bool contains(Vector2i coords) const;
  String get_city_id() const;
  bool set_city_id(String city_id) const;
  Array get_available_improvements() const;
  String get_region_id() const;
  Dictionary convert_to_dictionary(const hs::proto::region::Improvement& improvement) const;
  Ref<PnlObject> get_pnl_statement(Ref<RulesetObject> ruleset) const;
  Dictionary get_jobs(Ref<RulesetObject> ruleset_object) const;

  // user data wrappers
  double get_data_numeric(Vector2i coords,String key) const noexcept;
  double set_data_numeric(Vector2i coords,String key, double value);
  bool has_data_numeric(Vector2i coords,String key) const noexcept;
  String get_data_string(Vector2i coords,const String& key) const noexcept;
  String set_data_string(Vector2i coords,String key, String value);
  bool has_data_string(Vector2i coords,const String& key) const noexcept;

  // signals
  void emit_signals_for_cell(Vector2i coords, int flags) const;

  //Array get_region_improvements() const;

  static QRSCoords cast_qrs(Vector2i coords) {
    return QRSCoords{
      QRSCoordinateSystem::QAxis{coords.x},
      QRSCoordinateSystem::RAxis{coords.y}
    };
  }

};
