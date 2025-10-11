#include "region_object.hpp"

#include <godot_cpp/core/object.hpp>
#include <ui/godot/module/utils/cast_qrs.hpp>

#define SETTER_GETTER_FLOAT(name, name_upper)                              \
  bool RegionObject::set_##name(Vector2i coords, double value) const {     \
    if (!region_) {                                                        \
      return false;                                                        \
    }                                                                      \
                                                                           \
    const auto qrs_coords = cast_qrs(coords);                              \
                                                                           \
    auto success = region_->Set##name_upper(qrs_coords, value);            \
                                                                           \
    if (success) {                                                         \
      emit_signals_for_cell(coords, 0);                                    \
    }                                                                      \
                                                                           \
    return success;                                                        \
  }                                                                        \
                                                                           \
  double RegionObject::get_##name(Vector2i coords) const {                 \
    if (!region_) {                                                        \
      return 0.0;                                                          \
    }                                                                      \
                                                                           \
    const auto qrs_coords = cast_qrs(coords);                              \
    if (!region_->GetSurface().Contains(qrs_coords)) {                     \
      return 0.0;                                                          \
    }                                                                      \
                                                                           \
    const auto result =                                                    \
        region_->GetSurface().GetCell(qrs_coords).Get##name_upper();       \
    if (region_->GetId() == StringName("rgn_0")) {                         \
      spdlog::info("Region {} Getting height at {}: {}",                   \
                   static_cast<void*>(region_.get()), qrs_coords, result); \
    }                                                                      \
    return result;                                                         \
  }

#define ERR_FAIL_NULL_TARGET_REGION(region, result) \
  ERR_FAIL_NULL_V_MSG(region, result, ERR_MSG_REGION_IS_NULL)
#define ERR_FAIL_NULL_REGION(result) \
  ERR_FAIL_NULL_TARGET_REGION(region_, result)

namespace hs::godot {

static constexpr const char* ERR_MSG_REGION_IS_NULL =
    "null-containing region object";

void RegionObject::_bind_methods() {
  ScopeMixin::_bind_methods<RegionObject>();

  ClassDB::bind_method(D_METHOD("get_dimensions"),
                       &RegionObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_id"), &RegionObject::get_region_id);
  ClassDB::bind_method(D_METHOD("get_info"), &RegionObject::get_info);
  ClassDB::bind_method(D_METHOD("get_city_id"), &RegionObject::get_city_id);
  ClassDB::bind_method(D_METHOD("set_city_id", "city_id"),
                       &RegionObject::set_city_id);
  ClassDB::bind_method(D_METHOD("get_cell_info", "coords"),
                       &RegionObject::get_cell_info);
  ClassDB::bind_method(D_METHOD("get_cell", "coords"), &RegionObject::get_cell);
  ClassDB::bind_method(D_METHOD("contains", "coords"), &RegionObject::contains);
  // ClassDB::bind_method(D_METHOD("set_biome", "coords", "biome"),
  // &RegionObject::set_biome);
  ClassDB::bind_method(D_METHOD("set_feature", "coords", "feature"),
                       &RegionObject::set_feature);
  ClassDB::bind_method(D_METHOD("set_improvement", "coords", "improvement"),
                       &RegionObject::set_improvement);
  ClassDB::bind_method(D_METHOD("get_available_improvements"),
                       &RegionObject::get_available_improvements);
  ClassDB::bind_method(D_METHOD("get_pnl_statement", "ruleset"),
                       &RegionObject::get_pnl_statement);
  ClassDB::bind_method(D_METHOD("get_jobs", "ruleset"),
                       &RegionObject::get_jobs);

  /* TODO: RM ?
  ClassDB::bind_method(D_METHOD("set_data_string", "coords", "key", "value"),
  &RegionObject::set_data_string);
  ClassDB::bind_method(D_METHOD("has_data_string", "coords", "key" ),
  &RegionObject::has_data_string);
  ClassDB::bind_method(D_METHOD("get_data_string", "coords", "key" ),
  &RegionObject::get_data_string);
  ClassDB::bind_method(D_METHOD("set_data_numeric", "coords", "key", "value"),
  &RegionObject::set_data_numeric);
  ClassDB::bind_method(D_METHOD("has_data_numeric", "coords", "key" ),
  &RegionObject::has_data_numeric);
  ClassDB::bind_method(D_METHOD("get_data_numeric", "coords", "key" ),
  &RegionObject::get_data_numeric);
  */
  // ClassDB::bind_method(D_METHOD("get_height", "coords" ),
  // &RegionObject::get_height); ClassDB::bind_method(D_METHOD("set_height",
  // "coords", "height"  ), &RegionObject::set_height);
  // ClassDB::bind_method(D_METHOD("get_temperature", "coords" ),
  // &RegionObject::get_temperature);
  // ClassDB::bind_method(D_METHOD("set_temperature", "coords", "temperature" ),
  // &RegionObject::set_temperature);
  // ClassDB::bind_method(D_METHOD("get_precipitation", "coords" ),
  // &RegionObject::get_precipitation);
  // ClassDB::bind_method(D_METHOD("set_precipitation", "coords",
  // "precipitation"  ), &RegionObject::set_precipitation);
  ClassDB::bind_method(D_METHOD("get_region_id"),
                       &RegionObject::get_region_id);  // deprecated

  // iterations
  ClassDB::bind_method(D_METHOD("foreach", "callback"), &RegionObject::foreach);

  // signals
  ADD_SIGNAL(MethodInfo("region_changed", PropertyInfo(Variant::RECT2I, "area"),
                        PropertyInfo(Variant::INT, "flags")));
}

ScopePtr RegionObject::GetScope() const {
  ERR_FAIL_NULL_REGION(ScopePtr{});

  return region_->GetScope();
}

Rect2i RegionObject::get_dimensions() const {
  auto result = Rect2i{Vector2i{0, 0}, Vector2i{1, 1}};
  ERR_FAIL_NULL_REGION(result);

  result = Rect2i{Vector2i{region_->GetSurface().q_start().ToUnderlying(),
                           region_->GetSurface().r_start().ToUnderlying()},
                  Vector2i{region_->GetSurface().q_size().ToUnderlying(),
                           region_->GetSurface().r_size().ToUnderlying()}};

  return result;
}

Dictionary RegionObject::get_cell_info(Vector2i coords) const {
  if (!region_) {
    return {};
  }
  auto qrs_coords = cast_qrs(coords);

  if (!region_->GetSurface().Contains(qrs_coords)) {
    return {};
  }

  auto& cell = region_->GetSurface().GetCell(qrs_coords);

  // Fill result
  Dictionary result;
  result["feature"] = cell.GetFeature();
  result["improvement"] = convert_to_dictionary(cell.GetImprovement());
  /* TODO: RM
  result["biome"] = cell.GetBiome();
  result["height"] = cell.GetHeight();
  result["temperature"] = cell.GetTemperature();
  result["precipitation"] = cell.GetPrecipitation();
  */

  return result;
}

Dictionary RegionObject::convert_to_dictionary(
    const hs::proto::region::Improvement& improvement) const {
  Dictionary result;
  if (improvement.type().empty()) {
    return result;
  }
  result["id"] = improvement.id();
  result["type"] = StringName(improvement.type().c_str());

  return result;
}

/*
bool RegionObject::set_biome(Vector2i coords, String biome) const
{
  if(!region_) {
    return false;
  }
  auto qrs_coords = cast_qrs(coords);

  // region.SetBiome will check coordinates
  auto success = region_->SetBiome(qrs_coords, biome);
  if( success) {
    emit_signals_for_cell(coords, 0);
  }

  return success;
}
*/

#if 0
TODO: RM
bool RegionObject::set_height(Vector2i coords, double height) const {
    if(!region_) {
        return false;
    }

  const auto qrs_coords = cast_qrs(coords);

  auto success = region_->SetHeight(qrs_coords, height);

  if( success) {
    emit_signals_for_cell(coords, 0);
  }

  return success;
}

double RegionObject::get_height(Vector2i coords) const {
    if(!region_) {
        return 0.0;
    }

  const auto qrs_coords = cast_qrs(coords);
  if (!region_->GetSurface().Contains(qrs_coords)) {
      return 0.0;
  }

  return region_->GetSurface().GetCell(qrs_coords).GetHeight();
}
#endif

bool RegionObject::set_feature(Vector2i coords, String feature) const {
  if (!region_) {
    return false;
  }
  auto qrs_coords = cast_qrs(coords);

  if (!region_->GetSurface().Contains(qrs_coords)) {
    return false;
  }

  auto success = region_->SetFeature(qrs_coords, feature);

  if (success) {
    emit_signals_for_cell(coords, 0);
  }

  return success;
}

bool RegionObject::set_improvement(Vector2i coords, String improvement) const {
  if (!region_) {
    return false;
  }
  auto qrs_coords = cast_qrs(coords);

  // Region object will perform checks that coords are valid and so
  // on.
  // However, it will not perform checks that this improvement follows
  // the ruleset
  auto success = region_->SetImprovement(qrs_coords, improvement);

  if (success) {
    emit_signals_for_cell(coords, 0);
  }

  return success;
}

Dictionary RegionObject::get_info() const {
  ERR_FAIL_NULL_REGION(Dictionary{});
  return RegionObject::make_region_info(*region_);
}

Dictionary RegionObject::make_region_info(const Region& region) {
  Dictionary result;
  // Build top biome
  {
    Array top_biome_result;
    auto top_biome = region.GetTopKBiomes(3);
    for (auto& [biome, count] : top_biome) {
      top_biome_result.append(count);
      top_biome_result.append(String(biome));
    }
    result["top_biome"] = std::move(top_biome_result);
  }
  result["city_id"] = region.GetCityId();
  result["region_id"] = region.GetId();
  /* TODO: RM
  const auto height_range = region.GetHeightRange();
  result["min_height"] = height_range.first;
  result["max_height"] = height_range.second;
  const auto temperature_range = region.GetTemperatureRange();
  result["min_temperature"] = temperature_range.first;
  result["max_temperature"] = temperature_range.second;
  const auto precipitation_range = region.GetPrecipitationRange();
  result["min_precipitation"] = precipitation_range.first;
  result["max_precipitation"] = precipitation_range.second;
  */

  return result;
}

Array RegionObject::get_available_improvements() const { return {}; }

bool RegionObject::contains(Vector2i coords) const {
  ERR_FAIL_NULL_REGION(false);
  auto qrs_coords = cast_qrs(coords);

  return region_->GetSurface().Contains(qrs_coords);
}

String RegionObject::get_region_id() const {
  ERR_FAIL_NULL_REGION(String{});
  return region_->GetId();
}

String RegionObject::get_city_id() const {
  ERR_FAIL_NULL_REGION(String{});

  return region_->GetCityId();
}

bool RegionObject::set_city_id(String city_id) const {
  ERR_FAIL_NULL_REGION(false);

  return region_->SetCityId(city_id);
}

Ref<PnlObject> RegionObject::get_pnl_statement(
    Ref<RulesetObject> ruleset) const {
  ERR_FAIL_NULL_REGION(Ref<PnlObject>{});
  ERR_FAIL_NULL_V_MSG(ruleset, Ref<PnlObject>{},
                      "null-containing ruleset object");

  Ref<PnlObject> result(
      memnew(PnlObject(region_->BuildPnlStatement(ruleset->GetRuleSet()))));

  return result;
}

// TODO: We can move this method to godot...
Dictionary RegionObject::get_jobs(Ref<RulesetObject> ruleset_object) const {
  Dictionary result;
  auto surface = region_->GetSurface();
  const auto& ruleset = ruleset_object->GetRuleSet();

  absl::flat_hash_map<std::string, size_t> data;

  for (auto qr_coords : region_->GetImprovedCells()) {
    DEBUG_VERIFY(surface.Contains(qr_coords));
    auto& cell = surface.GetCell(qr_coords);

    if (!cell.HasImprovement()) {
      spdlog::error("Incorrect cells_with_improvements_, missing improvement");
      continue;
    }

    auto& improvement = cell.GetImprovement();

    // Get its type
    const hs::proto::ruleset::RegionImprovement* improvement_ruleset =
        ruleset.FindRegionImprovementByType(improvement.type());

    if (improvement_ruleset == nullptr) {
      spdlog::error("Can\'t get ruleset info for improvement {}",
                    improvement.type());
      continue;
    }

    // Get all jobs
    for (auto& [k, v] : improvement_ruleset->jobs()) {
      data[k] += v;
    }
  }

  // Convert to Godot
  for (auto& [k, v] : data) {
    result[String{k.c_str()}] = v;
  }

  return result;
}

/*

double RegionObject::get_data_numeric(Vector2i coords,const StringName& key)
const noexcept { auto qrs_coords = cast_qrs(coords); return
region_->GetDataNumeric(qrs_coords, key);
}

double RegionObject::set_data_numeric(Vector2i coords,const StringName& key,
double value) { auto qrs_coords = cast_qrs(coords); return
region_->SetDataNumeric(qrs_coords, key, value);
}

bool RegionObject::has_data_numeric(Vector2i coords, const StringName& key)
const noexcept { ERR_FAIL_NULL_V_MSG(region_, false, "null-containing region
object"); auto qrs_coords = cast_qrs(coords); return
region_->HasDataNumeric(qrs_coords, key);
}

String RegionObject::get_data_string(Vector2i coords,const StringName& key)
const noexcept { auto qrs_coords = cast_qrs(coords); return
region_->GetDataString(qrs_coords,key);
}

String RegionObject::set_data_string(Vector2i coords, const StringName& key,
String value) { auto qrs_coords = cast_qrs(coords); return
region_->SetDataString(qrs_coords,key, value);
}

bool RegionObject::has_data_string(Vector2i coords,const StringName& key) const
noexcept { ERR_FAIL_NULL_V_MSG(region_, false, "null-containing region object");
  auto qrs_coords = cast_qrs(coords);
    return region_->HasDataString(qrs_coords,key);
}
*/

void RegionObject::emit_signals_for_cell(Vector2i coords, int flags) const {
  Rect2i area{coords, Vector2i{1, 1}};
  const_cast<RegionObject*>(this)->emit_signal("region_changed", area, flags);
}

void RegionObject::foreach (const Callable& callback) {
  using Cell = typename Region::Surface::Cell;
  region_->GetSurface().foreach ([&callback](QRSCoords coords, Cell& cell) {
    callback.call(coords.q().ToUnderlying(), coords.r().ToUnderlying());
  });
}

Ref<CellObject> RegionObject::get_cell(Vector2i coords) {
  ERR_FAIL_NULL_REGION(Ref<CellObject>{});
  auto qrs_coords = cast_qrs(coords);

  Ref<CellObject> result(memnew(CellObject(region_, qrs_coords)));
  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create cell object");
  return result;
}

/* TODO: RM
SETTER_GETTER_FLOAT(height, Height);
SETTER_GETTER_FLOAT(temperature, Temperature);
SETTER_GETTER_FLOAT(precipitation, Precipitation);
*/
}  // namespace hs::godot
