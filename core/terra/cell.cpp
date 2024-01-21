#include "cell.hpp"

namespace hs::terra {

flatbuffers::Offset<fbs::Cell> SerializeTo(const Cell& source, fbs::CellBuilder& builder, ::flatbuffers::FlatBufferBuilder& fbb)
{
  builder.add_terrain(fbb.CreateSharedString(source.terrain));

  return builder.Finish();
}

Cell ParseFrom( const fbs::Cell& fbs_class) {
  Cell result;
  if(fbs_class.terrain()) {
    result.SetTerrain(fbs_class.terrain()->string_view());
  }
}

}
