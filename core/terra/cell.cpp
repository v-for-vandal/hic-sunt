#include "cell.hpp"

namespace hs::terra {

flatbuffers::Offset<fbs::Cell> SerializeTo(const Cell& source, ::flatbuffers::FlatBufferBuilder& fbb)
{
  auto terrain_offset = fbb.CreateSharedString(source.GetTerrain());

  fbs::CellBuilder builder(fbb);
  builder.add_terrain(terrain_offset);

  return builder.Finish();
}

Cell ParseFrom( const fbs::Cell& fbs_class, serialize::To<Cell>) {
  Cell result;
  if(fbs_class.terrain()) {
    result.SetTerrain(fbs_class.terrain()->string_view());
  }

  return result;
}

}
