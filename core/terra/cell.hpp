#pragma once


namespace hi::terra {
  enum class Terrain {
    Plains = 0,
    Hills,
    Mountain
  };

  class Cell {

  public:
    Terrain GetTerrain() const { return terrain_type_; }
    auto SetTerrain(Terrain terrain) { terrain_type_ = terrain; }

  private:
    Terrain terrain_type_;
  };
}
