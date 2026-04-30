#pragma once

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include "spdlog/spdlog.h"

namespace hs::scope {

/** \brief Base class for all variables
 */
template <typename BaseTypes = StdBaseTypes>
class VariableBase {
    public:
        size_t GetModificationTime() const noexcept { return modification_time_; }
    protected:
        void UpdateModificationTime(size_t modification_time)
        {
        if (modification_time != 0) {
            if (modification_time >= modification_time_) {
                modification_time_ = modification_time;
            } else {
                spdlog::warn("Attempt to decrease modification time from {} to {}",
                    modification_time_, modification_time
                );
            }
        }
        }

    private:
        size_t modification_time_{0};
};

}
