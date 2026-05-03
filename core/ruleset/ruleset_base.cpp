#include "ruleset_base.hpp"

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <ruleset/effect.pb.h>
#include <ruleset/region_improvements.pb.h>
#include <ruleset/variables.pb.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <vector>

namespace hs::ruleset {

namespace {

template <typename Proto>
bool ReadFromFile(const std::filesystem::path& path, Proto& proto_object) {
  std::ifstream f(path);
  if (!f.is_open()) {
    spdlog::warn("Can't open ruleset file {}", path.string());
    return false;
  }

  google::protobuf::io::IstreamInputStream pb_input_stream{&f};
  if (!google::protobuf::TextFormat::Parse(&pb_input_stream, &proto_object)) {
    spdlog::warn("Can't parse ruleset file {}", path.string());
    return false;
  }

  return true;
}

std::vector<std::filesystem::path> FilterValidRuleRoots(
    const std::vector<std::filesystem::path>& roots) {
  std::vector<std::filesystem::path> result;
  result.reserve(roots.size());

  for (const auto& root : roots) {
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
      spdlog::warn("Ruleset path {} is not a directory and will be ignored",
                   root.string());
      continue;
    }
    result.push_back(root);
  }

  return result;
}

std::vector<std::filesystem::path> CollectRuleFiles(
    const std::vector<std::filesystem::path>& roots,
    const std::filesystem::path& subdirectory) {
  std::vector<std::filesystem::path> result;

  for (const auto& root : roots) {
    const auto rules_root = root / subdirectory;
    if (!std::filesystem::exists(rules_root) ||
        !std::filesystem::is_directory(rules_root)) {
      continue;
    }

    std::vector<std::filesystem::path> local_files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(rules_root)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      if (entry.path().extension() != ".txt") {
        continue;
      }
      local_files.push_back(entry.path());
    }

    std::sort(local_files.begin(), local_files.end());
    result.insert(result.end(), local_files.begin(), local_files.end());
  }

  return result;
}

template <typename Element>
int FindById(const auto& repeated_field, const std::string& id) {
  for (int idx = 0; idx < repeated_field.size(); ++idx) {
    if (repeated_field.Get(idx).id() == id) {
      return idx;
    }
  }
  return -1;
}

template <typename RepeatedField, typename Element>
void UpsertRepeatedField(RepeatedField* target, const Element& source,
                         const std::filesystem::path& file_path,
                         const char* object_type_name) {
  const int existing_idx = FindById<Element>(*target, source.id());
  if (existing_idx >= 0) {
    spdlog::info("Overriding {} {} from file {}", object_type_name, source.id(),
                 file_path.string());
    (*target)[existing_idx] = source;
  } else {
    *target->Add() = source;
  }
}

void ApplyFile(proto::ruleset::RegionImprovements& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::RegionImprovements parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& improvement : parsed.improvements()) {
    UpsertRepeatedField(target.mutable_improvements(), improvement, file_path,
                        "region improvement");
  }
}

void ApplyFile(proto::ruleset::Biomes& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::Biomes parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& biome : parsed.biomes()) {
    UpsertRepeatedField(target.mutable_biomes(), biome, file_path, "biome");
  }
  for (const auto& biome_feature : parsed.biome_features()) {
    UpsertRepeatedField(target.mutable_biome_features(), biome_feature,
                        file_path, "biome feature");
  }
}

void ApplyFile(proto::ruleset::Resources& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::Resources parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& resource : parsed.resources()) {
    UpsertRepeatedField(target.mutable_resources(), resource, file_path,
                        "resource");
  }
}

void ApplyFile(proto::ruleset::Jobs& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::Jobs parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& job : parsed.jobs()) {
    UpsertRepeatedField(target.mutable_jobs(), job, file_path, "job");
  }
}

void ApplyFile(proto::ruleset::Projects& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::Projects parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& project : parsed.projects()) {
    UpsertRepeatedField(target.mutable_projects(), project, file_path,
                        "project");
  }
}

void ApplyFile(proto::render::Rendering& target,
               const std::filesystem::path& file_path) {
  proto::render::Rendering parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& atlas_rendering : parsed.atlas_rendering()) {
    UpsertRepeatedField(target.mutable_atlas_rendering(), atlas_rendering,
                        file_path, "atlas rendering");
  }
}

void ApplyFile(proto::ruleset::Variables& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::Variables parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& variable : parsed.variables()) {
    UpsertRepeatedField(target.mutable_variables(), variable, file_path,
                        "variable");
  }
}

void ApplyFile(std::vector<proto::ruleset::effect::Effect>& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::effect::Effects parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& effect : parsed.effects()) {
    const auto fit = std::ranges::find_if(target, [&effect](const auto& existing) {
      return existing.id() == effect.id();
    });
    if (fit != target.end()) {
      spdlog::info("Overriding effect {} from file {}", effect.id(),
                   file_path.string());
      *fit = effect;
    } else {
      target.push_back(effect);
    }
  }
}

template <typename Target>
void LoadOrderedFilesInto(Target& target,
                         const std::vector<std::filesystem::path>& files) {
  for (const auto& file : files) {
    ApplyFile(target, file);
  }
}

}  // namespace

bool RuleSetBase::Load(const std::vector<std::filesystem::path>& paths,
                       ErrorsCollection& /*errors*/) {
  Clear();

  const auto valid_paths = FilterValidRuleRoots(paths);

  const auto improvement_files = CollectRuleFiles(valid_paths, improvements_dir);
  const auto biome_files = CollectRuleFiles(valid_paths, biomes_dir);
  const auto resource_files = CollectRuleFiles(valid_paths, resources_dir);
  const auto job_files = CollectRuleFiles(valid_paths, jobs_dir);
  const auto rendering_files = CollectRuleFiles(valid_paths, rendering_dir);
  const auto project_files = CollectRuleFiles(valid_paths, projects_dir);
  const auto variable_files = CollectRuleFiles(valid_paths, variable_definitions_dir);
  const auto effect_files = CollectRuleFiles(valid_paths, effects_dir);

  LoadOrderedFilesInto(improvements_, improvement_files);
  LoadOrderedFilesInto(biomes_, biome_files);
  LoadOrderedFilesInto(resources_, resource_files);
  LoadOrderedFilesInto(jobs_, job_files);
  LoadOrderedFilesInto(rendering_, rendering_files);
  LoadOrderedFilesInto(projects_, project_files);
  LoadOrderedFilesInto(variable_definitions_, variable_files);
  LoadOrderedFilesInto(effects_, effect_files);

  return true;
}

void RuleSetBase::Clear() {
  biomes_.Clear();
  improvements_.Clear();
  resources_.Clear();
  rendering_.Clear();
  jobs_.Clear();
  projects_.Clear();
  variable_definitions_.Clear();
  effects_.clear();
}

}  // namespace hs::ruleset
