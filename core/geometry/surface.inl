#pragma once

namespace hs::geometry {

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem>::Surface(
      SurfaceShape shape
  ):
  shape_(shape),
  allocation_size_(GetAllocationSize(shape)),
  data_size_(allocation_size_.first * allocation_size_.second),
  data_storage_(new Cell[data_size_]),
  cells_(
    CellsArrayView<Cell>(data_storage_.get(), allocation_size_.first, allocation_size_.second ),
    shape
    )
{
}

template<typename Cell, typename CoordinateSystem>
bool SurfaceView<Cell, CoordinateSystem>::operator==(const SurfaceView& other) const {
  if(this == &other) {
    return true;
  }

  if(shape_ != other.shape_) {
      return false;
  }

  // compare cells
  for(auto q = q_start(); q != q_end(); q++) {
    for(auto r = r_start(); r != r_end(); r++) {
      Coords coords{q,r};
      if(Contains(coords) != other.Contains(coords)) {
        SPDLOG_TRACE("Different contains outcome for coords {}", coords);
        return false;
      }

      if( Contains(coords) ) {
        if( GetCell(coords) != other.GetCell(coords)) {
          SPDLOG_TRACE("Cell differs for coords {}", coords);
          return false;
        }
      }
    }
  }

  return true;

}

template<typename Cell, typename CoordinateSystem>
std::pair<size_t, size_t> Surface<Cell, CoordinateSystem>::GetAllocationSize(const SurfaceShape& shape) noexcept {
    auto box = shape.BoundingBox();

    return std::make_pair(size_t(box.q_size().ToUnderlying()), size_t(box.r_size().ToUnderlying()));
}


template<typename Cell, typename CoordinateSystem>
inline Surface<Cell, CoordinateSystem> ParseFrom(const auto& proto_source, serialize::To<Surface<Cell, CoordinateSystem>>)
{
    SurfaceShape<CoordinateSystem> shape = ParseFrom(
        proto_source.shape(),
        serialize::To<SurfaceShape<CoordinateSystem>>{}
        );

  Surface<Cell, CoordinateSystem> result{
      shape
  };




  const auto& cells_handle = proto_source.cells();
  if(static_cast<int>(result.data_size()) < cells_handle.size()) {
    spdlog::error(
      "Not enough storage in Surface. Surface data size: {} proto cells size: {}",
      result.data_size(),
      cells_handle.size()
      );
    throw std::runtime_error("Not enough storage in Surface");
  }

  size_t out_it = 0;
  for(const auto& elem : cells_handle) {
    result.GetCell(out_it) = ParseFrom(elem, serialize::To<Cell>{});
    out_it++;
  }

  return result;
}

/* For proto serialization */
template<typename Cell, typename CoordinateSystem>
auto SerializeTo(const Surface<Cell, CoordinateSystem>& source, auto& proto_destination)
{
  proto_destination.Clear();

  auto* mutable_cells_handler = proto_destination.mutable_cells();
  mutable_cells_handler->Reserve(source.data_size());

  for(size_t idx = 0; idx < source.data_size(); ++idx) {
    auto* new_element = mutable_cells_handler->Add();
    SerializeTo(source.GetCell(idx), *new_element);
  }

  SerializeTo(source.GetShape(), *proto_destination.mutable_shape());
}

}
