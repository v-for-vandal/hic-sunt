#include "ruleset_base.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>
#include <ruleset/effect.pb.h>
#include <ruleset/improvements.pb.h>
#include <ruleset/variables.pb.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <c4/std/string.hpp>
#include <fstream>
#include <ryml.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace hs::ruleset {

namespace {

bool IsRulesetFileExtension(const std::filesystem::path& path) {
  const auto extension = path.extension().string();
  return extension == ".txt" || extension == ".yaml" || extension == ".yml";
}

bool IsYamlFileExtension(const std::filesystem::path& path) {
  const auto extension = path.extension().string();
  return extension == ".yaml" || extension == ".yml";
}

std::filesystem::path WithoutExtension(std::filesystem::path path) {
  path.replace_extension();
  return path;
}

std::string RymlSubstringToString(c4::csubstr value) { return std::string(value.str, value.len); }

class RymlError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

[[noreturn]] void OnRymlBasicError(c4::csubstr message, const ryml::ErrorDataBasic& /*error_data*/,
                                   void* /*user_data*/) {
  throw RymlError(RymlSubstringToString(message));
}

[[noreturn]] void OnRymlParseError(c4::csubstr message, const ryml::ErrorDataParse& error_data,
                                   void* /*user_data*/) {
  throw RymlError(
      fmt::format("{} at line {}", RymlSubstringToString(message), error_data.ymlloc.line + 1));
}

[[noreturn]] void OnRymlVisitError(c4::csubstr message, const ryml::ErrorDataVisit& /*error_data*/,
                                   void* /*user_data*/) {
  throw RymlError(RymlSubstringToString(message));
}

ryml::Callbacks MakeRymlCallbacks() {
  ryml::Callbacks callbacks;
  callbacks.set_error_basic(&OnRymlBasicError);
  callbacks.set_error_parse(&OnRymlParseError);
  callbacks.set_error_visit(&OnRymlVisitError);
  return callbacks;
}

bool ValidateRymlNode(const ryml::Tree& tree, ryml::id_type node, std::string& error) {
  if (tree.has_anchor(node)) {
    error = "YAML anchors are not supported in ruleset files";
    return false;
  }
  if (tree.is_ref(node)) {
    error = "YAML aliases are not supported in ruleset files";
    return false;
  }
  if (tree.has_key_tag(node) || tree.has_val_tag(node)) {
    error = "YAML tags are not supported in ruleset files";
    return false;
  }

  if (tree.is_stream(node)) {
    if (tree.num_children(node) > 1) {
      error = "multiple YAML documents are not supported";
      return false;
    }
  }

  std::unordered_set<std::string> keys;
  for (auto child = tree.first_child(node); child != ryml::NONE; child = tree.next_sibling(child)) {
    if (tree.is_map(node)) {
      if (!tree.has_key(child)) {
        error = "YAML mapping keys must be scalars in ruleset files";
        return false;
      }
      const auto key = RymlSubstringToString(tree.key(child));
      if (!keys.insert(key).second) {
        error = fmt::format("duplicate YAML mapping key: {}", key);
        return false;
      }
    }
    if (!ValidateRymlNode(tree, child, error)) {
      return false;
    }
  }

  return true;
}

bool ReadTextProtoFromFile(const std::filesystem::path& path,
                           google::protobuf::Message& proto_object) {
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

bool ReadYamlProtoFromFile(const std::filesystem::path& path,
                           google::protobuf::Message& proto_object) {
  std::ifstream input(path);
  if (!input.is_open()) {
    spdlog::warn("Can't open ruleset file {}", path.string());
    return false;
  }

  std::stringstream buffer;
  buffer << input.rdbuf();
  const std::string yaml = buffer.str();
  const std::string filename = path.string();

  ryml::Tree tree(MakeRymlCallbacks());
  try {
    ryml::parse_in_arena(c4::to_csubstr(filename), c4::to_csubstr(yaml), &tree);
  } catch (const RymlError& ex) {
    spdlog::warn("Can't parse ruleset YAML file {}: {}", path.string(), ex.what());
    return false;
  }

  std::string error;
  if (!ValidateRymlNode(tree, tree.root_id(), error)) {
    spdlog::warn("Can't parse ruleset YAML file {}: {}", path.string(), error);
    return false;
  }

  ryml::id_type json_root = tree.root_id();
  if (tree.is_stream(json_root) && tree.num_children(json_root) == 1) {
    json_root = tree.first_child(json_root);
  }

  std::string json;
  try {
    json = ryml::emitrs_json<std::string>(tree, json_root);
  } catch (const RymlError& ex) {
    spdlog::warn("Can't convert ruleset YAML file {} to JSON: {}", path.string(), ex.what());
    return false;
  }

  google::protobuf::util::JsonParseOptions options;
  const auto status = google::protobuf::util::JsonStringToMessage(json, &proto_object, options);
  if (!status.ok()) {
    spdlog::warn("Can't parse ruleset YAML file {} as protobuf JSON: {}", path.string(),
                 status.ToString());
    return false;
  }

  return true;
}

template <typename Proto>
bool ReadFromFile(const std::filesystem::path& path, Proto& proto_object) {
  if (IsYamlFileExtension(path)) {
    return ReadYamlProtoFromFile(path, proto_object);
  }
  return ReadTextProtoFromFile(path, proto_object);
}

std::vector<std::filesystem::path> FilterValidRuleRoots(
    const std::vector<std::filesystem::path>& roots) {
  std::vector<std::filesystem::path> result;
  result.reserve(roots.size());

  for (const auto& root : roots) {
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
      spdlog::warn("Ruleset path {} is not a directory and will be ignored", root.string());
      continue;
    }
    result.push_back(root);
  }

  return result;
}

std::vector<std::filesystem::path> CollectRuleFiles(const std::vector<std::filesystem::path>& roots,
                                                    const std::filesystem::path& subdirectory) {
  std::vector<std::filesystem::path> result;

  for (const auto& root : roots) {
    const auto rules_root = root / subdirectory;
    if (!std::filesystem::exists(rules_root) || !std::filesystem::is_directory(rules_root)) {
      continue;
    }

    std::vector<std::filesystem::path> local_files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(rules_root)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      if (!IsRulesetFileExtension(entry.path())) {
        continue;
      }
      local_files.push_back(entry.path());
    }

    std::sort(
        local_files.begin(), local_files.end(), [&rules_root](const auto& lhs, const auto& rhs) {
          const auto lhs_order_key = WithoutExtension(std::filesystem::relative(lhs, rules_root));
          const auto rhs_order_key = WithoutExtension(std::filesystem::relative(rhs, rules_root));
          if (lhs_order_key != rhs_order_key) {
            return lhs_order_key < rhs_order_key;
          }
          return lhs.extension() < rhs.extension();
        });

    std::vector<std::filesystem::path> filtered_files;
    for (std::size_t idx = 0; idx < local_files.size();) {
      const auto conflict_parent = local_files[idx].parent_path();
      const auto conflict_stem = local_files[idx].stem();
      std::size_t end = idx + 1;
      while (end < local_files.size() && local_files[end].parent_path() == conflict_parent &&
             local_files[end].stem() == conflict_stem) {
        end++;
      }

      if (end - idx == 1) {
        filtered_files.push_back(local_files[idx]);
      } else {
        std::vector<std::string> paths;
        paths.reserve(end - idx);
        for (std::size_t conflict_idx = idx; conflict_idx < end; ++conflict_idx) {
          paths.push_back(local_files[conflict_idx].string());
        }
        spdlog::warn(
            "Ruleset files with the same name but different extensions are not allowed "
            "in one directory and will be ignored: {}",
            paths);
      }
      idx = end;
    }

    result.insert(result.end(), filtered_files.begin(), filtered_files.end());
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
                         const std::filesystem::path& file_path, const char* object_type_name) {
  const int existing_idx = FindById<Element>(*target, source.id());
  if (existing_idx >= 0) {
    spdlog::info("Overriding {} {} from file {}", object_type_name, source.id(),
                 file_path.string());
    (*target)[existing_idx] = source;
  } else {
    *target->Add() = source;
  }
}

void ApplyFile(proto::ruleset::Improvements& target, const std::filesystem::path& file_path) {
  proto::ruleset::Improvements parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& improvement : parsed.improvements()) {
    UpsertRepeatedField(target.mutable_improvements(), improvement, file_path,
                        "region improvement");
  }
}

void ApplyFile(proto::ruleset::Biomes& target, const std::filesystem::path& file_path) {
  proto::ruleset::Biomes parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& biome : parsed.biomes()) {
    UpsertRepeatedField(target.mutable_biomes(), biome, file_path, "biome");
  }
  for (const auto& biome_feature : parsed.biome_features()) {
    UpsertRepeatedField(target.mutable_biome_features(), biome_feature, file_path, "biome feature");
  }
}

void ApplyFile(proto::ruleset::Resources& target, const std::filesystem::path& file_path) {
  proto::ruleset::Resources parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& resource : parsed.resources()) {
    UpsertRepeatedField(target.mutable_resources(), resource, file_path, "resource");
  }
}

void ApplyFile(proto::ruleset::Jobs& target, const std::filesystem::path& file_path) {
  proto::ruleset::Jobs parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& job : parsed.jobs()) {
    UpsertRepeatedField(target.mutable_jobs(), job, file_path, "job");
  }
}

void ApplyFile(proto::ruleset::Projects& target, const std::filesystem::path& file_path) {
  proto::ruleset::Projects parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& project : parsed.projects()) {
    UpsertRepeatedField(target.mutable_projects(), project, file_path, "project");
  }
}

void ApplyFile(proto::render::Rendering& target, const std::filesystem::path& file_path) {
  proto::render::Rendering parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& atlas_rendering : parsed.atlas_rendering()) {
    UpsertRepeatedField(target.mutable_atlas_rendering(), atlas_rendering, file_path,
                        "atlas rendering");
  }
}

void ApplyFile(proto::ruleset::Variables& target, const std::filesystem::path& file_path) {
  proto::ruleset::Variables parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& variable : parsed.variables()) {
    UpsertRepeatedField(target.mutable_variables(), variable, file_path, "variable");
  }
}

void ApplyFile(std::vector<proto::ruleset::effect::Effect>& target,
               const std::filesystem::path& file_path) {
  proto::ruleset::effect::Effects parsed;
  if (!ReadFromFile(file_path, parsed)) {
    return;
  }

  for (const auto& effect : parsed.effects()) {
    const auto fit = std::ranges::find_if(
        target, [&effect](const auto& existing) { return existing.id() == effect.id(); });
    if (fit != target.end()) {
      spdlog::info("Overriding effect {} from file {}", effect.id(), file_path.string());
      *fit = effect;
    } else {
      target.push_back(effect);
    }
  }
}

template <typename Target>
void LoadOrderedFilesInto(Target& target, const std::vector<std::filesystem::path>& files) {
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
