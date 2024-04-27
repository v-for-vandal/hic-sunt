#pragma once

#include <iostream> // TODO: RM

namespace hs::geometry {

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem>::Surface(
    typename SCS::QAxis q_start ,
    typename SCS::QAxis q_end  ,
    typename SCS::RAxis r_start ,
    typename SCS::RAxis r_end  ,
    typename SCS::SAxis s_start ,
    typename SCS::SAxis s_end
  ):
  data_size_(make_size_sane(q_start, q_end) * make_size_sane(r_start, r_end)),
  data_storage_(new Cell[data_size_]),
  cells_(
    CellsArrayView<Cell>(data_storage_.get(), make_size_sane(q_start, q_end),
      make_size_sane(r_start, r_end) ),
    q_start, r_start, s_start, s_end

    )
{

}

template<typename Cell, typename CoordinateSystem>
bool SurfaceView<Cell, CoordinateSystem>::operator==(const SurfaceView& other) const {
  if(this == &other) {
    return true;
  }

#define _HICSUNT_CMP_VAL(func) \
  if( func() != other.func()) {\
    SPDLOG_TRACE(#func " not equal, {} vs {}", func(), other.func());\
    return false;\
  }

  _HICSUNT_CMP_VAL(q_start)
  _HICSUNT_CMP_VAL(q_end)
  _HICSUNT_CMP_VAL(r_start)
  _HICSUNT_CMP_VAL(r_end)
  _HICSUNT_CMP_VAL(s_start)
  _HICSUNT_CMP_VAL(s_end)

#undef _HICSUNT_CMP_VAL

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
inline Surface<Cell, CoordinateSystem> ParseFrom(const auto& proto_source, serialize::To<Surface<Cell, CoordinateSystem>>)
{
  Surface<Cell, CoordinateSystem> result{
    typename CoordinateSystem::QAxis{proto_source.q_start()},
    typename CoordinateSystem::QAxis{proto_source.q_end()},
    typename CoordinateSystem::RAxis{proto_source.r_start()},
    typename CoordinateSystem::RAxis{proto_source.r_end()},
    typename CoordinateSystem::SAxis{proto_source.s_start()},
    typename CoordinateSystem::SAxis{proto_source.s_end()}
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

  proto_destination.set_q_start(source.q_start().ToUnderlying());
  proto_destination.set_q_end(source.q_end().ToUnderlying());
  proto_destination.set_r_start(source.r_start().ToUnderlying());
  proto_destination.set_r_end(source.r_end().ToUnderlying());
  proto_destination.set_s_start(source.s_start().ToUnderlying());
  proto_destination.set_s_end(source.s_end().ToUnderlying());
}

}
