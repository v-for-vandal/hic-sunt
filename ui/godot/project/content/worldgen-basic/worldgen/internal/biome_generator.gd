extends WorldGeneratorModuleInterface

const _TEMPERATURE_RANGE = Vector2i(-20, 40) # celsius
const _TEMPERATURE_FLUCTUATION = 5

const _HEIGHT_RANGE = Vector2i(-8000, 8000) # meters
const _MOUNTAIN_EXTRA_HEIGHT_RANGE = Vector2i(3000, 6000) # meters

const _PRECIPATION_RANGE = Vector2i(0, 400)

func make_biome_rect(temp_start:int, temp_end:int, percipation_start:int, percipation_end:int) -> Rect2i:
	return Rect2i( temp_start, percipation_start, (temp_end-temp_start), (percipation_end - percipation_start))
	
# TODO: Move to proto
var biome_maps = [
	{
		make_biome_rect(_TEMPERATURE_RANGE.x, -10, 0, _PRECIPATION_RANGE.y+1) : "core.biome.snow",
		make_biome_rect(-10, 0, 0,  _PRECIPATION_RANGE.y+1) : "core.biome.tundra",
		make_biome_rect(0, 5, 0, 20) : "core.biome.temperate_grassland",
		make_biome_rect(0, 5, 20, _PRECIPATION_RANGE.y + 1) : "core.biome.taiga", # boreal forest
		make_biome_rect(5, 20, 0, 40) : "core.biome.temperate_grassland", #	
		make_biome_rect(5, 15, 40, 200) : "core.biome.temperate_forest",
		make_biome_rect(5, 15, 200, _PRECIPATION_RANGE.y + 1) : "core.biome.rainforest",
		make_biome_rect(15, 20, 40, 120) : "core.biome.shrubland",
		make_biome_rect(15, 20, 120, 230) : "core.biome.temperate_forest",
		make_biome_rect(15, 20, 230, _PRECIPATION_RANGE.y + 1) : "core.biome.rainforest",
		make_biome_rect(20, 30, 0, 100 ) : "core.biome.desert", # subtropical_desert
		make_biome_rect(20, 30, 100, 230 ) : "core.biome.savanna", # subtropical_desert
		make_biome_rect(20, 30, 230, _PRECIPATION_RANGE.y + 1 ) : "core.biome.tropical_rainforest", # subtropical_desert
		make_biome_rect(30, _TEMPERATURE_RANGE.y+1, 0, _PRECIPATION_RANGE.y+1 ) : "core.biome.desert", # subtropical_desert

	}
]

func _wrap_x(i: float) -> float:
	# Wrapping will be done within plane object itself
	return i
	
	
func _wrap_y(j: float) -> float:
	return j

func _get_biome(temperature: int, precipation: int) -> String:
	var point := Vector2i(temperature, precipation)
	
	for biome_map : Dictionary in biome_maps:
		for rect : Rect2i in biome_map:
			if rect.has_point(point):
				return biome_map[rect]
				
	print("Can't find biome for (temp, prcp): ", point)
	return &"core.biome.unknown"
	
#func _get_biome_at_point(i: float, j:float) -> String:
	#i = _wrap_x(i)
	#j = _wrap_y(j)
	#var temp = _get_temperature_at_point(i, j)
	#var height = _get_height_at_point(i, j)
	#var precipation = _get_precipation_at_point(i, j)
	#
	#if height < 0:
		#return "core.biome.ocean"
		#
	#return _get_biome(temp, precipation)
