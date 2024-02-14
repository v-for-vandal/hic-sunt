#include "ruleset_object.hpp"

void RulesetObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_all_region_improvements"), &RulesetObject::get_all_region_improvements);
  ClassDB::bind_method(D_METHOD("get_terrain_types"), &RulesetObject::get_terrain_types);
}

Array RulesetObject::get_terrain_types() const {
  Array result;


  const auto& terrain = ruleset_.GetTerrain();

  for(const auto& terrain_type : terrain.terrain_types()) {
    result.append(convert_terrain_type(terrain_type));
  }

  return result;
}

Dictionary RulesetObject::convert_terrain_type(const hs::proto::ruleset::TerrainType& terrain_type) {
  Dictionary result;
  result["id"] = terrain_type.id().c_str();
  result["render"] = convert_render(terrain_type.render());

  return result;
}

Dictionary RulesetObject::convert_render(const hs::proto::render::AtlasRender& render) {
  Dictionary result;
  result["resource"] = render.resource().c_str();
  //render["source_id"] = terrain_type.render().resource();
  {
    int x = 0;
    int y = 0;
    if (render.atlas_coords_size() > 0) {
      x = render.atlas_coords(0);
    }
    if (render.atlas_coords_size() > 0) {
      y = render.atlas_coords(1);
    }
    result["atlas_coords"] = Vector2i(x,y);
  }

  return result;
}


Array RulesetObject::get_all_region_improvements() const {
  const auto& improvements = ruleset_.GetRegionImprovements();

  Array result;

  for(auto& improvement: improvements.improvements()) {
    Dictionary godot_improvement_info;
    godot_improvement_info["id"] = improvement.id().c_str();

    result.append(std::move(godot_improvement_info));
  }

  return result;
}



