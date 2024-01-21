#pragma once

#include <string>
#include <string_view>


#include <core/utils/serialize.hpp>

#include <fbs/world_generated.hpp>

namespace hs::terra {
  enum class Terrain {
    Plains = 0,
    Hills,
    Mountain
  };

  class Cell {

  public:
    std::string_view GetTerrain() const { return terrain_type_; }
    void SetTerrain(std::string_view terrain) { terrain_type_ = terrain; }

  private:
    std::string terrain_type_;
  };

flatbuffers::Offset<fbs::Cell> SerializeTo(const Cell& source, fbs::CellBuilder& builder, ::flatbuffers::FlatBufferBuilder& fbb);
Cell ParseFrom( const fbs::Cell& fbs_class);

}
