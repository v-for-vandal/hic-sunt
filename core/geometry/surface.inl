#pragma once

namespace hs::geometry {

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem>::Surface(typename SCS::QDelta q_size, typename SCS::RDelta r_size):
  data_size_(make_size_sane(q_size) * make_size_sane(r_size)),
  data_storage_(new Cell[data_size_]),
  cells_(CellsArrayView<Cell>(data_storage_.get(), make_size_sane(q_size), make_size_sane(r_size)))
{

}

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem>::Surface(SCSize size):
  Surface(size.q(), size.r())
{
}

/* Deprecated in favor of protobuf
template<typename Cell, typename CoordinateSystem>
auto SerializeTo(const Surface<Cell, CoordinateSystem>& source, ::flatbuffers::FlatBufferBuilder& fbb, auto to)
{
  // Deduce cell fb builder type by our builder type
  // first, get fb class by fb builder
  using BuilderType = std::decay_t<typename decltype(to)::Target>;

  using SurfaceFbClass = typename BuilderType::Table;

  // fb class must have method 'cells'
  // it will be something like
  // const ::flatbuffers::Vector<::flatbuffers::Offset<hs::fbs::Cell>> *cells() const
  using VectorOfOfsettOfCell = std::decay_t<decltype(*std::declval<SurfaceFbClass>().cells())>;

  // Now, we can actually get 'Cell' from vector because flatbuffer internally 
  // has this nice function IndirectHelper and Vector::return_type will be
  // exactly Cell
  using CellType = std::remove_cv_t<std::remove_pointer_t<std::decay_t<typename VectorOfOfsettOfCell::return_type>>>;

  std::function<flatbuffers::Offset<CellType>(size_t)> serialize_cell_fn = [&source, &fbb](size_t idx) {
    flatbuffers::Offset<CellType> offset = SerializeTo(source.GetCell(idx), fbb);
    return offset;
    };

  auto cells_offset = fbb.CreateVector<::flatbuffers::Offset<CellType>>(
    source.data_size(),
    serialize_cell_fn
    );

  BuilderType builder(fbb);
  builder.add_cells(cells_offset);
  builder.add_q_size(source.q_size().ToUnderlying());
  builder.add_r_size(source.r_size().ToUnderlying());

  return builder.Finish();
}
*/

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
