#include "ruleset_object.hpp"

void RulesetObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_all_region_improvements"), &RulesetObject::get_all_region_improvements);
  ClassDB::bind_method(D_METHOD("get_biomes"), &RulesetObject::get_biomes);
  ClassDB::bind_method(D_METHOD("get_all_resources"), &RulesetObject::get_all_resources);
  ClassDB::bind_method(D_METHOD("get_atlas_render"), &RulesetObject::get_atlas_render);
}

Array RulesetObject::get_biomes() const {
  Array result;

  const auto& biome = ruleset_.GetBiomes();

  for(const auto& biome_type : biome.biomes()) {
    result.append(convert_biome_type(biome_type));
  }

  return result;
}

Dictionary RulesetObject::convert_biome_type(const hs::proto::ruleset::Biome& biome_type) {
  Dictionary result;
  result["id"] = biome_type.id().c_str();

  return result;
}

Dictionary RulesetObject::get_atlas_render() const {
  Dictionary result;

  for(auto& render_info : ruleset_.GetRendering().atlas_rendering()) {
    result[render_info.id().c_str()] = convert_render(render_info);
  }

  return result;
}

Dictionary RulesetObject::convert_render(const hs::proto::render::AtlasRender& render) {
  Dictionary result;
  result["resource"] = render.resource().c_str();
  result["source_id"] = render.source_id();
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

Array RulesetObject::get_all_resources() const {

  const auto& resources = ruleset_.GetResources();
  Array result;

  for(const auto& res: resources.resources()) {
    Dictionary resource_info;
    resource_info["id"] = res.id().c_str();
    result.append(std::move(resource_info));
  }

  return result;

}


