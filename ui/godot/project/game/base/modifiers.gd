extends Node

const GEOGRAPHY_HEIGHT := &"core.geography.height"
const GEOGRAPHY_SOUTH_POLE_DISTANCE := &"core.geography.sp_distance"
const GEOGRAPHY_NORTH_POLE_DISTANCE := &"core.geography.np_distance"

const ECOSYSTEM_PRECIPITATION := &"core.ecosystem.precipitation"
const ECOSYSTEM_TEMPERATURE := &"core.ecosystem.temperature"
const ECOSYSTEM_BIOME = &"core.ecosystem.biome"
# Max level that world generators should use for biomes. Everything above
# is reserved for objects that want to override biome
const ECOSYSTEM_BIOME_MAX_LEVEL = 100
