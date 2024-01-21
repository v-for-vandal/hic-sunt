#pragma once

namespace geometry {

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem>::Surface(CSR::QDelta q_size, SCS::RDelta r_size):
  data_size_(q_size.ToUnderlying() * r_size.ToUnderlying()),
  data_storage_(new Cell[data_size_]),
  cells_(CellsArrayView(data_storage_.get(), q_size, r_size))
{

}

}
