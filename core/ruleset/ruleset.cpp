#include "ruleset.hpp"

#include <fstream>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <spdlog/spdlog.h>

#include <ruleset/region_improvements.pb.h>

namespace hs::ruleset {

namespace {

bool ReadFromFile(const std::filesystem::path& path, auto& proto_object, utils::ErrorsCollection& errors) {
  {
    std::ifstream f(path);
    if (!f.is_open()) {
      // for now, just throw an exception
      errors.AddError(
        utils::ErrorMessage{fmt::format("Can't open file {}", path.c_str())});
      return false;
    }
    google::protobuf::io::IstreamInputStream pb_input_stream{&f};

    return google::protobuf::TextFormat::Parse(&pb_input_stream, &proto_object);
  }

}

}

bool RuleSet::Load(const std::filesystem::path& path,
  ErrorsCollection& errors) {
  // TODO: Skip files that are not present
    // TODO: Must actually merge rules, not overwrite them
  SPDLOG_INFO("Parsing improvements file");
  bool success = ReadFromFile( path / improvements_file, improvements_, errors);
  if(!success) {
    return false;
  }

  SPDLOG_INFO("Parsing biomes file");
  success = ReadFromFile( path / biomes_file, biomes_, errors);
  if(!success) {
    return false;
  }

  SPDLOG_INFO("Parsing resources file");
  success = ReadFromFile( path / resources_file, resources_, errors);
  if(!success) {
    return false;
  }

  SPDLOG_INFO("Parsing jobs file");
  success = ReadFromFile( path / jobs_file, jobs_, errors);
  if(!success) {
    return false;
  }

  SPDLOG_INFO("Parsing rendering file");
  success = ReadFromFile( path / rendering_file, rendering_, errors);
  if(!success) {
    return false;
  }

  // Building hashes
  for(int idx = 0; idx < improvements_.improvements_size(); ++idx) {
    const auto& improvement = improvements_.improvements(idx);
    improvements_by_type_.try_emplace(improvement.id(), idx);
  }

  for(int idx = 0; idx < jobs_.jobs_size(); ++idx) {
    const auto& job = jobs_.jobs(idx);
    jobs_by_type_.try_emplace(job.id(), idx);
  }



  return true;

}

void RuleSet::Clear() {
  biomes_.Clear();
  improvements_.Clear();
  resources_.Clear();
  rendering_.Clear();
  jobs_.Clear();
  improvements_by_type_.clear();
  jobs_by_type_.clear();
}

const proto::ruleset::RegionImprovement* RuleSet::FindRegionImprovementByType(
    utils::StringTokenCRef improvement_type_id) const
{
  auto fit = improvements_by_type_.find(improvement_type_id);
  if( fit != improvements_by_type_.end()) {
    const auto result_idx = fit->second;
    return &improvements_.improvements(result_idx);
  }

  return nullptr;
}

const proto::ruleset::Job* RuleSet::FindJobByType(
    utils::StringTokenCRef job_type_id) const
{
  auto fit = jobs_by_type_.find(job_type_id);
  if( fit != jobs_by_type_.end()) {
    const auto result_idx = fit->second;
    return &jobs_.jobs(result_idx);
  }

  return nullptr;
}

}
