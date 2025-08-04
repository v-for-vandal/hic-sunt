namespace hs::region {

template<typename BaseTypes>
auto PnlStatement<BaseTypes>::GetTotal() const -> ResourceFlow {
  ResourceFlow result{profit_};

  for(const auto& [key, value] : losses_) {
    result[key] += value;
  }

  return result;
}

}

