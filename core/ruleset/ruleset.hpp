#pragma once

#include <string_view>
#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <core/ruleset/ruleset_base.hpp>

#include <core/utils/error_message.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::ruleset {

template<typename BaseTypes = StdBaseTypes>
class RuleSet :  RuleSetBase {
public:
  using ErrorsCollection = utils::ErrorsCollection;
  using StringId = StdBaseTypes::StringId;
  void Clear();
  // Adds data to ruleset
  bool Load(const std::filesystem::path& path, ErrorsCollection& errors);

  const proto::ruleset::RegionImprovement* FindRegionImprovementByType(
    const StringId& improvement_type_id) const;

  const proto::ruleset::Job* FindJobByType(
    const StringId& job_type_id) const;

  const proto::ruleset::Project* FindProjectByType(
    const StringId& project_type_id) const;


private:

  absl::flat_hash_map<StringId, size_t> improvements_by_type_;
  absl::flat_hash_map<StringId, size_t> jobs_by_type_;
  absl::flat_hash_map<StringId, size_t> projects_by_type_;
};

}

#include "ruleset.inl"
