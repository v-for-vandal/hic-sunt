
namespace hs::utils {

template<typename BaseTypes>
bool Comb<BaseTypes>::operator==(const Comb& other) const noexcept
{
  if(count_.size() != other.count_.size()) {
    SPDLOG_TRACE("not equal, by size");
    return false;
  }

  for(const auto& [k,v] : count_) {
    auto other_fit = other.count_.find(k);
    if(other_fit == other.count_.end()) {
      SPDLOG_TRACE("key {} not found in other", k);
      return false;
    }

    if(v != other_fit->second) {
      SPDLOG_TRACE("value differ for key \"{}\": {} vs {}", k, v, other_fit->second);
      return false;
    }
  }

  return true;
}

}
