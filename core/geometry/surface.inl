#pragma once

namespace geometry {

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem>::Surface(typename SCS::QDelta q_size, typename SCS::RDelta r_size):
  data_size_(q_size.ToUnderlying() * r_size.ToUnderlying()),
  data_storage_(new Cell[data_size_]),
  cells_(CellsArrayView<Cell>(data_storage_.get(), q_size, r_size))
{

}

}
