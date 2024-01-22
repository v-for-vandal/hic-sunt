#pragma once

namespace hs::geometry {

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem>::Surface(typename SCS::QDelta q_size, typename SCS::RDelta r_size):
  data_size_(q_size.ToUnderlying() * r_size.ToUnderlying()),
  data_storage_(new Cell[data_size_]),
  cells_(CellsArrayView<Cell>(data_storage_.get(), q_size.ToUnderlying(), r_size.ToUnderlying()))
{

}

template<typename Cell, typename CoordinateSystem>
auto SerializeTo(const Surface<Cell, CoordinateSystem>& source, ::flatbuffers::FlatBufferBuilder& fbb, auto to)
{
  // Deduce cell fb builder type by our builder type
  // first, get fb class by fb builder
  using BuilderType = std::decay_t<typename decltype(to)::Target>;
  BuilderType builder(fbb);

  using SurfaceFbClass = typename BuilderType::Table;

  // fb class must have method 'cells'
  // it will be something like
  // const ::flatbuffers::Vector<::flatbuffers::Offset<hs::fbs::Cell>> *cells() const
  using VectorOfOfsettOfCell = std::decay_t<decltype(*std::declval<SurfaceFbClass>().cells())>;

  // Now, we can actually get 'Cell' from vector because flatbuffer internally 
  // has this nice function IndirectHelper and Vector::return_type will be
  // exactly Cell
  using CellType = std::remove_cv_t<std::remove_pointer_t<std::decay_t<typename VectorOfOfsettOfCell::return_type>>>;

  // Now, we can get CellBuilder
  using CellBuilderType = typename CellType::Builder;

  std::function<flatbuffers::Offset<CellType>(size_t)> serialize_cell_fn = [&source, &fbb](size_t idx) {
    CellBuilderType cell_builder(fbb);
    flatbuffers::Offset<CellType> offset = SerializeTo(source.GetCell(idx), cell_builder, fbb);
    return offset;
    };

  auto cells_offset = fbb.CreateVector<::flatbuffers::Offset<CellType>>(
    source.data_size(),
    serialize_cell_fn
    );

  builder.add_cells(cells_offset);
  builder.add_q_size(source.q_size().ToUnderlying());
  builder.add_r_size(source.r_size().ToUnderlying());

  return builder.Finish();
}

template<typename Cell, typename CoordinateSystem>
inline Surface<Cell, CoordinateSystem> ParseFrom(const auto& fbs_class, serialize::To<Surface<Cell, CoordinateSystem>>)
{
  Surface<Cell, CoordinateSystem> result{
    typename CoordinateSystem::QDelta{fbs_class.q_size()},
    typename CoordinateSystem::RDelta{fbs_class.r_size()}};

  auto cells = fbs_class.cells();
  if(cells) {
    size_t out_it = 0;
    for(auto fbs_elem : *cells) {
      if(fbs_elem) {
        result.GetCell(out_it) = ParseFrom(*fbs_elem, serialize::To<Cell>{});
      }
      out_it++;
    }

  }

  return result;
}

}
