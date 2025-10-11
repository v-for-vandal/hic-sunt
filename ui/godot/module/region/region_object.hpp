#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <memory>
#include <ui/godot/module/game/pnl_object.hpp>
#include <ui/godot/module/game/ruleset_object.hpp>
#include <ui/godot/module/region/cell_object.hpp>
#include <ui/godot/module/region/region.hpp>
#include <ui/godot/module/terra/world.hpp>

namespace hs::godot {

using namespace ::godot;

class RegionObject : public RefCounted, public ScopeMixin {
  GDCLASS(RegionObject, RefCounted);

 public:
  RegionObject() {}
  RegionObject(std::shared_ptr<Region> region) : region_(std::move(region)) {}

  using QRSCoordinateSystem = World::QRSCoordinateSystem;
  using QRSCoords = World::QRSCoords;

  void _init() {}

  static void _bind_methods();

  static Dictionary make_region_info(const Region& region);

 private:
  std::shared_ptr<Region> region_;

 public:
  ScopePtr GetScope() const;
  Ref<ScopeObject> get_scope() const { return ScopeMixin::get_scope(); }
  // iterates over cells and gets N most frequent values of given variable
  // For values with eqaul frequency, sort by alphabet
  TypedArray<StringName> get_topn_string_values(StringName variable,
                                                int N) const;
  Rect2i get_dimensions() const;
  Dictionary get_cell_info(Vector2i coords) const;
  bool set_feature(Vector2i coords, String feature) const;
  bool set_improvement(Vector2i coords, String improvement) const;
  bool contains(Vector2i coords) const;
  String get_city_id() const;
  bool set_city_id(String city_id) const;
  Array get_available_improvements() const;
  String get_region_id() const;
  Dictionary convert_to_dictionary(
      const hs::proto::region::Improvement& improvement) const;
  Ref<PnlObject> get_pnl_statement(Ref<RulesetObject> ruleset) const;
  Dictionary get_jobs(Ref<RulesetObject> ruleset_object) const;
  Dictionary get_info() const;

  Ref<CellObject> get_cell(Vector2i coords);

  int get_numeric_value(const StringName& variable);
  String get_string_value(const StringName& variable);

  int add_numeric_modifier(const StringName& variables, const StringName& key,
                           float add, float mult);

  // environmental properties
  /*
  bool set_biome(Vector2i coords, String biome) const;
  bool set_height(Vector2i coords, double height) const;
  double get_height(Vector2i coords) const;
  bool set_temperature(Vector2i coords, double temperature) const;
  double get_temperature(Vector2i coords) const;
  bool set_precipitation(Vector2i coords, double precipitation) const;
  double get_precipitation(Vector2i coords) const;
  */

  // user data wrappers
  /*
  double get_data_numeric(Vector2i coords,const StringName& key) const noexcept;
  double set_data_numeric(Vector2i coords,const StringName& key, double value);
  bool has_data_numeric(Vector2i coords, const StringName& key) const noexcept;
  String get_data_string(Vector2i coords,const StringName& key) const noexcept;
  String set_data_string(Vector2i coords, const StringName& key, String value);
  bool has_data_string(Vector2i coords,const StringName& key) const noexcept;
  */

  // signals
  void emit_signals_for_cell(Vector2i coords, int flags) const;

  // Array get_region_improvements() const;

  void foreach (const Callable& callback);
};

}  // namespace hs::godot
