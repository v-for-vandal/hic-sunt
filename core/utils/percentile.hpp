#pragma once

namespace hs::utils {

template<typename T>
struct NumericAggregationInfo {
  T min{};
  T max{};
  T avg{};
  T median{};
  size_t count{0};
};

template <typename T>
class PercentileBuilder {
public:
  void Account(T value) {
    values_.push_back(value);
    sum_ += value;
  }

  NumericAggregationInfo<T> GetAggregationInfo() {
    std::ranges::sort(values_);

    if (values_.empty()) {
      return {};
    }

    const size_t count = values_.size();

    NumericAggregationInfo<T> result;
    result.min = *values_.begin();
    result.max = *values_.rbegin();

    result.avg = sum_ / count;
    result.count = count;
    result.median = (values_[count / 2] + values_[count-1 - count / 2] )/ 2.0;


    return result;
  }

private:
  std::vector<T> values_;
  double sum_{0};
};

}  // namespace hs::utils

