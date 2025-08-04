namespace hs::region {

template<typename BaseTypes>
Region<BaseTypes>::Region():
  Region("", 1)
{
}

template<typename BaseTypes>
Region<BaseTypes>::Region(const StringId& region_id, int radius):
  id_(region_id),
  surface_(
    geometry::HexagonSurface(radius)
    )
  {
    BuildEphemeral();
  }

template<typename BaseTypes>
bool Region<BaseTypes>::operator==(const Region& other) const {
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


template<typename BaseTypes>
bool Region<BaseTypes>::SetBiome(QRSCoords coords, const StringId& biome)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  if (BaseTypes::IsNullToken(biome)) {
      return false;
  }


  auto& cell = surface_.GetCell(coords);
  if(!BaseTypes::IsNullToken(cell.GetBiome())) {
    biome_count_.Remove(cell.GetBiome());
  }
  cell.SetBiome(biome);
  biome_count_.Add(biome);

  return true;
}

template<typename BaseTypes>
bool Region<BaseTypes>::SetHeight(QRSCoords coords, double height)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  // TODO: RM
  //if (height < 0) {
    spdlog::info("Region {} Setting height  at {}: {}",
      static_cast<void*>(this), coords, height);
  //}
  cell.SetHeight(height);
  height_minmax_.Account(height);

  return true;
}

template<typename BaseTypes>
bool Region<BaseTypes>::SetTemperature(QRSCoords coords, double temperature)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  cell.SetTemperature(temperature);
  temperature_minmax_.Account(temperature);

  return true;
}

template<typename BaseTypes>
bool Region<BaseTypes>::SetPrecipitation(QRSCoords coords, double precipitation)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  cell.SetPrecipitation(precipitation);
  precipitation_minmax_.Account(precipitation);

  return true;
}

template<typename BaseTypes>
bool Region<BaseTypes>::SetFeature(QRSCoords coords, const StringId& feature)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  cell.SetFeature(feature);
  return true;
}


template<typename BaseTypes>
bool Region<BaseTypes>::SetImprovement(QRSCoords coords, const StringId& improvement_type)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  if(BaseTypes::IsNullToken(improvement_type)){
    spdlog::error("can't use empty improvement type, use RemoveImprovement instead");
    return false;
  }

  auto& cell = surface_.GetCell(coords);

  proto::region::Improvement new_improvement;
  new_improvement.set_id(next_unique_id_++);
  new_improvement.set_type(BaseTypes::ToProtoString(improvement_type));
  cell.SetImprovement(new_improvement);

  cells_with_improvements_.insert(coords);

  return true;
}


template<typename BaseTypes>
bool Region<BaseTypes>::SetCityId(const StringId& city_id) {
  if(IsCity() && !BaseTypes::IsNullToken(city_id)) {
    spdlog::error("Can't set city_id in region where city is already present");
    return false;
  }

  city_id_ = city_id;
  return true;
}

template<typename BaseTypes>
double Region<BaseTypes>::GetDataNumeric(QRSCoords coords, const StringId& key) const noexcept
{
  if(!surface_.Contains(coords)) {
    return 0.0;
  }

  auto& cell = surface_.GetCell(coords);
  return cell.GetDataNumeric(key);
}

template<typename BaseTypes>
double Region<BaseTypes>::SetDataNumeric(QRSCoords coords, const StringId& key, double value) {
  if(!surface_.Contains(coords)) {
    return 0.0;
  }

  auto& cell = surface_.GetCell(coords);
  return cell.SetDataNumeric(key, value);

}

template<typename BaseTypes>
bool Region<BaseTypes>::HasDataNumeric(QRSCoords coords, const StringId& key) const noexcept {
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  return cell.HasDataNumeric(key);
}

template<typename BaseTypes>
auto Region<BaseTypes>::GetDataString(QRSCoords coords, const StringId& key) const noexcept -> const String&
{
  if(!surface_.Contains(coords)) {
      static String empty_value;
      return empty_value;
  }

  auto& cell = surface_.GetCell(coords);
  return cell.GetDataString(key);
}

template<typename BaseTypes>
auto Region<BaseTypes>::SetDataString(QRSCoords coords, const StringId& key, const StringId& value) -> const String& {
  if(!surface_.Contains(coords)) {
      static String empty_value;
        return empty_value;
  }

  auto& cell = surface_.GetCell(coords);
  return cell.SetDataString(key, value);

}

template<typename BaseTypes>
bool Region<BaseTypes>::HasDataString(QRSCoords coords, const StringId& key) const noexcept {
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  return cell.HasDataString(key);
}

template<typename BaseTypes>
void Region<BaseTypes>::BuildEphemeral() {
  biome_count_.clear();
  feature_count_.clear();
  cells_with_improvements_.clear();
  auto surface = surface_.view();
  for(auto q = surface.q_start(); q != surface.q_end(); ++q) {
    for(auto r = surface.r_start(); r != surface.r_end(); ++r) {
      QRSCoords coords(q,r);
      if(surface.Contains(coords)) {
        auto& cell = surface.GetCell(coords);
        biome_count_.Add(cell.GetBiome());
        feature_count_[cell.GetFeature()]++;
        if(cell.HasImprovement()) {
          cells_with_improvements_.insert(coords);
        }
        height_minmax_.Account(cell.GetHeight());
        temperature_minmax_.Account(cell.GetTemperature());
        precipitation_minmax_.Account(cell.GetPrecipitation());
      }
    }
  }

  ephemeral_ready_.set();
}

template<typename BaseTypes>
auto Region<BaseTypes>::BuildPnlStatement([[maybe_unused]] const ruleset::RuleSet<BaseTypes>& ruleset) const -> PnlStatement
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

template<typename BaseTypes>
void SerializeTo(const Region<BaseTypes>& source, proto::region::Region& target)
{
  target.Clear();
  SerializeTo(source.surface_, *target.mutable_surface());
  target.set_id(BaseTypes::ToProtoString(source.id_));
  target.set_city_id(BaseTypes::ToProtoString(source.city_id_));
  target.set_unique_id_counter(source.next_unique_id_);
}

template<typename BaseTypes>
Region<BaseTypes> ParseFrom(const proto::region::Region& region, serialize::To<Region<BaseTypes>>)
{
    using StringId = typename BaseTypes::StringId;
  Region<BaseTypes> result;
  if(region.has_surface()) {
    result.surface_ = ParseFrom(region.surface(), serialize::To<typename Region<BaseTypes>::Surface>{});
  }
  result.id_ = ParseFrom(region.id(), serialize::To<StringId>{});
  result.city_id_ = ParseFrom(region.city_id(), serialize::To<StringId>{});
  result.next_unique_id_ = region.unique_id_counter();

  // Restore non-persistent data
  result.BuildEphemeral();

  return result;
}
}
