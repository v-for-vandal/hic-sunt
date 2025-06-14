#include "ruleset_object.hpp"

#include <spdlog/spdlog.h>

namespace hs::godot {

void RulesetObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_all_region_improvements"), &RulesetObject::get_all_region_improvements);
  ClassDB::bind_method(D_METHOD("get_biomes"), &RulesetObject::get_biomes);
  ClassDB::bind_method(D_METHOD("get_all_resources"), &RulesetObject::get_all_resources);
  ClassDB::bind_method(D_METHOD("get_atlas_render"), &RulesetObject::get_atlas_render);
  ClassDB::bind_method(D_METHOD("get_improvement_info", "improvement_id"), &RulesetObject::get_improvement_info);
  ClassDB::bind_method(D_METHOD("get_job_info", "job_id"), &RulesetObject::get_job_info);
  ClassDB::bind_method(D_METHOD("get_project_info", "project_id"), &RulesetObject::get_project_info);
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

Dictionary RulesetObject::convert_improvement(const hs::proto::ruleset::RegionImprovement& improvement_type)
{
  Dictionary result;
  result["id"] = improvement_type.id().c_str();

  {
    Dictionary cost;
    for(const auto& [k,v] : improvement_type.cost().amounts()) {
      cost[k.c_str()] = v;
    }
    result["cost"] = std::move(cost);
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

Dictionary RulesetObject::convert_job(const hs::proto::ruleset::Job& job) {
  Dictionary result;
  result["id"] = job.id().c_str();

  Dictionary input;
  for(const auto& [k,v] : job.input()) {
    input[k.c_str()] = v;
  }

  Dictionary output;
  for(const auto& [k,v] : job.output()) {
    output[k.c_str()] = v;
  }

  result["input"] = std::move(input);
  result["output"] = std::move(output);

  return result;
}

Dictionary RulesetObject::convert_project(const hs::proto::ruleset::Project& project) {
  Dictionary result;
  result["id"] = project.id().c_str();
  result["script"] = project.script().c_str();

  return result;
}

Array RulesetObject::get_projects() const {
  const auto& projects = ruleset_.GetProjects();

  Array result;
  for(auto& project: projects.projects()) {
    result.append(convert_project(project));
  }

  return result;
}

Array RulesetObject::get_all_region_improvements() const {
  const auto& improvements = ruleset_.GetRegionImprovements();

  Array result;

  for(auto& improvement: improvements.improvements()) {
    result.append(convert_improvement(improvement));
  }

  return result;
}

Dictionary RulesetObject::get_improvement_info(String id) const {
  auto ascii_id = id.ascii();
  const auto* improvement = ruleset_.FindRegionImprovementByType(ascii_id.get_data());

  if(improvement == nullptr) {
    spdlog::error("Can't find improvement with id {}", ascii_id.get_data());
    return {};
  }

  return convert_improvement(*improvement);
}

Dictionary RulesetObject::get_job_info(String id) const {
  auto ascii_id = id.ascii();
  const auto* job = ruleset_.FindJobByType(ascii_id.get_data());
  if(job == nullptr) {
    spdlog::error("Can't find job with id {}", ascii_id.get_data());
    return {};
  }

  return convert_job(*job);
}

Dictionary RulesetObject::get_project_info(String id) const {
  auto ascii_id = id.ascii();
  const auto* project = ruleset_.FindProjectByType(ascii_id.get_data());
  if(project == nullptr) {
    spdlog::error("Can't find project with id {}", ascii_id.get_data());
    return {};
  }

  return convert_project(*project);
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

}
