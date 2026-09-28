#include "ruleset_base.hpp"

#include <fmt/format.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>
#include <ruleset/effect.pb.h>
#include <ruleset/improvements.pb.h>
#include <ruleset/variables.pb.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/anchor.h>
#include <yaml-cpp/eventhandler.h>
#include <yaml-cpp/parser.h>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>
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

class SafeYamlEventHandler final : public YAML::EventHandler {
 public:
  explicit SafeYamlEventHandler(const std::filesystem::path& path) : path_(path) {}

  bool IsValid() const { return error_.empty(); }
  const std::string& GetError() const { return error_; }

  void OnDocumentStart(const YAML::Mark& mark) override {
    document_count_++;
    if (document_count_ > 1) {
      Reject(mark, "multiple YAML documents are not supported");
    }
  }
  void OnDocumentEnd() override {}

  void OnNull(const YAML::Mark& mark, YAML::anchor_t anchor) override { CheckAnchor(mark, anchor); }

  void OnAlias(const YAML::Mark& mark, YAML::anchor_t /*anchor*/) override {
    Reject(mark, "YAML aliases are not supported in ruleset files");
  }

  void OnScalar(const YAML::Mark& mark, const std::string& tag, YAML::anchor_t anchor,
                const std::string& /*value*/) override {
    CheckAnchor(mark, anchor);
    CheckTag(mark, tag);
  }

  void OnSequenceStart(const YAML::Mark& mark, const std::string& tag, YAML::anchor_t anchor,
                       YAML::EmitterStyle::value /*style*/) override {
    CheckAnchor(mark, anchor);
    CheckTag(mark, tag);
  }
  void OnSequenceEnd() override {}

  void OnMapStart(const YAML::Mark& mark, const std::string& tag, YAML::anchor_t anchor,
                  YAML::EmitterStyle::value /*style*/) override {
    CheckAnchor(mark, anchor);
    CheckTag(mark, tag);
  }
  void OnMapEnd() override {}

 private:
  void CheckAnchor(const YAML::Mark& mark, YAML::anchor_t anchor) {
    if (anchor != YAML::NullAnchor) {
      Reject(mark, "YAML anchors are not supported in ruleset files");
    }
  }

  void CheckTag(const YAML::Mark& mark, const std::string& tag) {
    if (!tag.empty() && tag != "?" && tag != "!") {
      Reject(mark, fmt::format("YAML tags are not supported in ruleset files: {}", tag));
    }
  }

  void Reject(const YAML::Mark& mark, const std::string& message) {
    if (!error_.empty()) {
      return;
    }
    error_ = fmt::format("{} at {}:{}", message, path_.string(), mark.line + 1);
  }

  std::filesystem::path path_;
  int document_count_ = 0;
  std::string error_;
};

bool ValidateSafeYamlSyntax(const std::filesystem::path& path, std::istream& input) {
  try {
    YAML::Parser parser(input);
    SafeYamlEventHandler handler(path);
    while (parser) {
      parser.HandleNextDocument(handler);
      if (!handler.IsValid()) {
        spdlog::warn("Can't parse ruleset YAML file {}: {}", path.string(), handler.GetError());
        return false;
      }
    }
  } catch (const YAML::Exception& ex) {
    spdlog::warn("Can't parse ruleset YAML file {}: {}", path.string(), ex.what());
    return false;
  }
  return true;
}

nlohmann::json ScalarYamlToJson(const YAML::Node& node) {
  const std::string scalar = node.Scalar();

  if (node.Tag() == "!") {
    return scalar;
  }
  if (scalar == "null" || scalar == "~") {
    return nullptr;
  }
  if (scalar == "true") {
    return true;
  }
  if (scalar == "false") {
    return false;
  }

  try {
    std::size_t parsed_chars = 0;
    const auto int_value = std::stoll(scalar, &parsed_chars, 10);
    if (parsed_chars == scalar.size()) {
      return int_value;
    }
  } catch (const std::exception&) {
  }

  try {
    std::size_t parsed_chars = 0;
    const auto double_value = std::stod(scalar, &parsed_chars);
    if (parsed_chars == scalar.size()) {
      return double_value;
    }
  } catch (const std::exception&) {
  }

  return scalar;
}

std::string JoinStrings(const std::vector<std::string>& values, std::string_view separator) {
  std::string result;
  for (std::size_t idx = 0; idx < values.size(); ++idx) {
    if (idx != 0) {
      result += separator;
    }
    result += values[idx];
  }
  return result;
}

bool YamlToJson(const YAML::Node& node, nlohmann::json& target, std::string& error) {
  if (!node.Tag().empty() && node.Tag() != "?" && node.Tag() != "!") {
    error = fmt::format("YAML tags are not supported in ruleset files: {}", node.Tag());
    return false;
  }

  switch (node.Type()) {
    case YAML::NodeType::Null:
      target = nullptr;
      return true;
    case YAML::NodeType::Scalar:
      target = ScalarYamlToJson(node);
      return true;
    case YAML::NodeType::Sequence: {
      target = nlohmann::json::array();
      for (const auto& item : node) {
        nlohmann::json json_item;
        if (!YamlToJson(item, json_item, error)) {
          return false;
        }
        target.push_back(std::move(json_item));
      }
      return true;
    }
    case YAML::NodeType::Map: {
      target = nlohmann::json::object();
      std::unordered_set<std::string> keys;
      for (const auto& item : node) {
        if (!item.first.IsScalar()) {
          error = "YAML mapping keys must be scalars in ruleset files";
          return false;
        }

        const std::string key = item.first.Scalar();
        if (!keys.insert(key).second) {
          error = fmt::format("duplicate YAML mapping key: {}", key);
          return false;
        }

        nlohmann::json value;
        if (!YamlToJson(item.second, value, error)) {
          return false;
        }
        target[key] = std::move(value);
      }
      return true;
    }
    case YAML::NodeType::Undefined:
      error = "undefined YAML node is not supported in ruleset files";
      return false;
  }

  error = "unsupported YAML node type";
  return false;
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
  std::ifstream validation_input(path);
  if (!validation_input.is_open()) {
    spdlog::warn("Can't open ruleset file {}", path.string());
    return false;
  }
  if (!ValidateSafeYamlSyntax(path, validation_input)) {
    return false;
  }

  YAML::Node yaml_root;
  try {
    yaml_root = YAML::LoadFile(path.string());
  } catch (const YAML::Exception& ex) {
    spdlog::warn("Can't parse ruleset YAML file {}: {}", path.string(), ex.what());
    return false;
  }

  nlohmann::json json;
  std::string error;
  if (!YamlToJson(yaml_root, json, error)) {
    spdlog::warn("Can't convert ruleset YAML file {} to JSON: {}", path.string(), error);
    return false;
  }

  google::protobuf::util::JsonParseOptions options;
  const auto status =
      google::protobuf::util::JsonStringToMessage(json.dump(), &proto_object, options);
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
            JoinStrings(paths, ", "));
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
