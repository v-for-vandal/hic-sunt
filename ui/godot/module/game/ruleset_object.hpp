#pragma once

#include <memory>

#include <core/terra/world.hpp>

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class RulesetObject : public RefCounted {
  GDCLASS(RulesetObject, RefCounted);

public:
  RulesetObject() {}
  RulesetObject(hs::ruleset::RuleSet&& data):
    ruleset_(std::move(data)) {}


  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods();

  void SetRuleSet(hs::ruleset::RuleSet rules) {
    ruleset_ = std::move(rules);
  }

  const auto& GetRuleSet() const { return ruleset_; }

private:
  hs::ruleset::RuleSet ruleset_;

public:
  Dictionary get_atlas_render() const;
  Array get_all_region_improvements() const;
  Array get_biomes() const;
  Array get_all_resources() const;
  static Dictionary convert_biome_type(const hs::proto::ruleset::Biome& biome_type);
  static Dictionary convert_render(const hs::proto::render::AtlasRender& render);
};

