#pragma once

namespace hs::utils {

    template<typename T>
    class MinMax {
    public:
        T GetMin() const noexcept { return min_; }
        T GetMax() const noexcept { return max_; }

        std::pair<T,T> GetRange() const noexcept {
            return std::make_pair(min_, max_);
        }

        void Account(T value) {
            min_ = std::min(min_, value);
            max_ = std::max(max_, value);
        }

    private:
        T min_{std::numeric_limits<T>::max()};
        T max_{std::numeric_limits<T>::min()};
    };

}

