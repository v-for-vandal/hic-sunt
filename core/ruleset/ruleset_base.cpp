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

template <typename Proto>
std::vector<std::filesystem::path> CollectRuleFiles(
    const std::filesystem::path& root) {
  std::vector<std::filesystem::path> result;
  if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
    return result;
  }

  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (entry.path().extension() != ".txt") {
      continue;
    }
    result.push_back(entry.path());
  }

  std::sort(result.begin(), result.end());
  return result;
}

void MergeInto(proto::ruleset::RegionImprovements& target,
               const proto::ruleset::RegionImprovements& source) {
  target.mutable_improvements()->MergeFrom(source.improvements());
}

void MergeInto(proto::ruleset::Biomes& target,
               const proto::ruleset::Biomes& source) {
  target.mutable_biomes()->MergeFrom(source.biomes());
  target.mutable_biome_features()->MergeFrom(source.biome_features());
}

void MergeInto(proto::ruleset::Resources& target,
               const proto::ruleset::Resources& source) {
  target.mutable_resources()->MergeFrom(source.resources());
}

void MergeInto(proto::ruleset::Jobs& target,
               const proto::ruleset::Jobs& source) {
  target.mutable_jobs()->MergeFrom(source.jobs());
}

void MergeInto(proto::ruleset::Projects& target,
               const proto::ruleset::Projects& source) {
  target.mutable_projects()->MergeFrom(source.projects());
}

void MergeInto(proto::render::Rendering& target,
               const proto::render::Rendering& source) {
  target.mutable_atlas_rendering()->MergeFrom(source.atlas_rendering());
}

void MergeInto(proto::ruleset::Variables& target,
               const proto::ruleset::Variables& source) {
  target.mutable_variables()->MergeFrom(source.variables());
}

void MergeInto(std::vector<proto::ruleset::effect::Effect>& target,
               const proto::ruleset::effect::Effects& source) {
  target.reserve(target.size() + source.effects_size());
  for (const auto& effect : source.effects()) {
    target.push_back(effect);
  }
}

template <typename Proto>
void LoadDirectoryInto(const std::filesystem::path& directory, Proto& target) {
  for (const auto& file : CollectRuleFiles<Proto>(directory)) {
    Proto parsed;
    if (!ReadFromFile(file, parsed)) {
      continue;
    }
    MergeInto(target, parsed);
  }
}

void LoadDirectoryInto(const std::filesystem::path& directory,
                       std::vector<proto::ruleset::effect::Effect>& target) {
  for (const auto& file : CollectRuleFiles<proto::ruleset::effect::Effects>(directory)) {
    proto::ruleset::effect::Effects parsed;
    if (!ReadFromFile(file, parsed)) {
      continue;
    }
    MergeInto(target, parsed);
  }
}

}  // namespace

bool RuleSetBase::Load(const std::vector<std::filesystem::path>& paths,
                       ErrorsCollection& /*errors*/) {
  Clear();

  for (const auto& path : paths) {
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
      spdlog::warn("Ruleset path {} is not a directory and will be ignored",
                   path.string());
      continue;
    }

    LoadDirectoryInto(path / improvements_dir, improvements_);
    LoadDirectoryInto(path / biomes_dir, biomes_);
    LoadDirectoryInto(path / resources_dir, resources_);
    LoadDirectoryInto(path / jobs_dir, jobs_);
    LoadDirectoryInto(path / rendering_dir, rendering_);
    LoadDirectoryInto(path / projects_dir, projects_);
    LoadDirectoryInto(path / variable_definitions_dir, variable_definitions_);
    LoadDirectoryInto(path / effects_dir, effects_);
  }

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
