#include "region.hpp"

namespace hs::region {

Region::Region():
  Region(1)
{
}

Region::Region(int radius):
  id_(fmt::format("region_{}", next_id_++)),
  surface_(
    QRSCoordinateSystem::QAxis(-radius),
    QRSCoordinateSystem::QAxis(radius),
    QRSCoordinateSystem::RAxis(-radius),
    QRSCoordinateSystem::RAxis(radius),
    QRSCoordinateSystem::SAxis(-radius),
    QRSCoordinateSystem::SAxis(radius)
    )
  {
    BuildEphemeral();
  }

bool Region::operator==(const Region& other) const {
  if(this == &other) {
    return true;
  }

  if(id_ != other.id_) {
    SPDLOG_TRACE("id not equal, {} vs {}", id_, other.id_);
    return false;
  }

  if(city_id_ != other.city_id_) {
    SPDLOG_TRACE("id not equal, {} vs {}", city_id_, other.city_id_);
    return false;
  }

  if(surface_ != other.surface_) {
    SPDLOG_TRACE("surface not equal");
    return false;
  }

  if(biome_count_ != other.biome_count_) {
    SPDLOG_TRACE("biome_count_ comb not equal");
    return false;
  }

  if(feature_count_ != other.feature_count_) {
    SPDLOG_TRACE("feature_count_ not equal");
    return false;
  }

  return true;
}


bool Region::SetBiome(QRSCoords coords, std::string_view biome)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  std::string biome_str{biome};

  auto& cell = surface_.GetCell(coords);
  if(!cell.GetBiome().empty()) {
    biome_count_.Remove(std::string{cell.GetBiome()});
  }
  cell.SetBiome(biome);
  biome_count_.Add(biome_str);

  return true;
}

bool Region::SetFeature(QRSCoords coords, std::string_view feature)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  cell.SetFeature(feature);
  return true;
  //cell.
}


bool Region::SetImprovement(QRSCoords coords, std::string_view improvement_type)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  if(improvement_type.empty()){
    spdlog::error("can't use empty improvement type, use RemoveImprovement instead");
    return false;
  }

  auto& cell = surface_.GetCell(coords);

  proto::region::Improvement new_improvement;
  new_improvement.set_id(next_unique_id_++);
  new_improvement.set_type(improvement_type);
  cell.SetImprovement(new_improvement);

  cells_with_improvements_.insert(coords);

  return true;
}


bool Region::SetCityId(std::string_view city_id) {
  if(IsCity() && !city_id.empty()) {
    spdlog::error("Can't set city_id in region where city is already present");
    return false;
  }

  city_id_ = city_id;
  return true;
}

void Region::BuildEphemeral() {
  biome_count_.clear();
  feature_count_.clear();
  cells_with_improvements_.clear();
  auto surface = surface_.view();
  for(auto q = surface.q_start(); q != surface.q_end(); ++q) {
    for(auto r = surface.r_start(); r != surface.r_end(); ++r) {
      QRSCoords coords(q,r);
      if(surface.Contains(coords)) {
        auto& cell = surface.GetCell(coords);
        biome_count_.Add(std::string{cell.GetBiome()});
        feature_count_[std::string{cell.GetFeature()}]++;
        if(cell.HasImprovement()) {
          cells_with_improvements_.insert(coords);
        }
      }
    }
  }

  ephemeral_ready_.set();
}

PnlStatement Region::BuildPnlStatement([[maybe_unused]] const ruleset::RuleSet& ruleset) const
{
  PnlStatement result;

  // Go through every improvement
#if 0
  TODO: do something with this code, input/production system was replaced with
          jobs system
  for(auto qr_coords : cells_with_improvements_) {
    DEBUG_VERIFY(surface_.Contains(qr_coords));
    auto& cell = surface_.GetCell(qr_coords);

    if(!cell.HasImprovement()) {
      spdlog::error("Incorrect cells_with_improvements_, missing improvement");
      continue;
    }

    auto& improvement = cell.GetImprovement();

    // Get its type
    const proto::ruleset::RegionImprovement* improvement_ruleset =
      ruleset.FindRegionImprovementByType(improvement.type());

    if(improvement_ruleset == nullptr) {
      spdlog::error("Can\'t get ruleset info for improvement {}", improvement.type());
      continue;
    }

    // Get improvement pnl
    const auto& input = improvement_ruleset->input();
    const auto& production = improvement_ruleset->production();

    for(const auto& [resource_id, amount] : input.amounts()) {
      result.GetLosses()[resource_id] -= amount;
    }
    for(const auto& [resource_id, amount] : production.amounts()) {
      result.GetProfit()[resource_id] += amount;
    }
  }
#endif

  return result;
}

void SerializeTo(const Region& source, proto::region::Region& target)
{
  target.Clear();
  SerializeTo(source.surface_, *target.mutable_surface());
  target.set_id(source.id_);
  target.set_city_id(source.city_id_);
  target.set_unique_id_counter(source.next_unique_id_);
}

Region ParseFrom(const proto::region::Region& region, serialize::To<Region>)
{
  Region result;
  if(region.has_surface()) {
    result.surface_ = ParseFrom(region.surface(), serialize::To<Region::Surface>{});
  }
  result.id_ = region.id();
  result.city_id_ = region.city_id();
  result.next_unique_id_ = region.unique_id_counter();

  // Restore non-persistent data
  result.BuildEphemeral();

  return result;
}
}
