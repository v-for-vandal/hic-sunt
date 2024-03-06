#include "pnl_statement.hpp"

namespace hs::region {

ResourceFlow PnlStatement::GetTotal() const {
  ResourceFlow result{profit_};

  for(const auto& [key, value] : losses_) {
    result[key] += value;
  }

  return result;
}

}
