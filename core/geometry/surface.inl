#pragma once

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
inline Surface<Cell, CoordinateSystem> ParseFrom(const auto& proto_source, serialize::To<Surface<Cell, CoordinateSystem>>)
{
  Surface<Cell, CoordinateSystem> result{
    typename CoordinateSystem::QDelta{std::max<int>(1, proto_source.q_size())},
    typename CoordinateSystem::RDelta{std::max<int>(1, proto_source.r_size())}};

  const auto& cells_handle = proto_source.cells();
  size_t out_it = 0;
  for(auto elem : cells_handle) {
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

  proto_destination.set_q_size(source.q_size().ToUnderlying());
  proto_destination.set_r_size(source.r_size().ToUnderlying());
}

}
