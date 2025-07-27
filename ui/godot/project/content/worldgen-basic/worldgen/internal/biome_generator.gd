extends WorldGeneratorModuleInterface

const Config = preload("res://content/worldgen-basic/worldgen/internal/biome_gen_config.gd")

const _TEMPERATURE_RANGE = Vector2i(-20, 40) # celsius
const _TEMPERATURE_FLUCTUATION = 5

const _PRECIPATION_RANGE = Vector2i(0, 400)

var _config : Config
var _plane: PlaneObject
var _global_context : Dictionary[StringName, Variant]

func _init(plane: PlaneObject, config: Variant, global_context: Dictionary[StringName, Variant]) -> void:
	super(global_context)
	_config = config
	_plane = plane
	_global_context = global_context

func make_biome_rect(temp_start:int, temp_end:int, percipation_start:int, percipation_end:int) -> Rect2i:
	return Rect2i( temp_start, percipation_start, (temp_end-temp_start), (percipation_end - percipation_start))
	
# TODO: Move to proto
var biome_maps : Array[Dictionary]= [
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
		make_biome_rect(20, 30, 100, 230 ) : "core.biome.savanna", # savanna
		make_biome_rect(20, 30, 230, _PRECIPATION_RANGE.y + 1 ) : "core.biome.tropical_rainforest", # subtropical_desert
		make_biome_rect(30, _TEMPERATURE_RANGE.y+1, 0, _PRECIPATION_RANGE.y+1 ) : "core.biome.desert", # subtropical_desert

	}
]

func first_pass() -> void:
	
	var region_lambda := func(region_q: int, region_r: int, region: RegionObject) -> void:
		_region_first_pass(region, Vector2i(region_q, region_r))
		
	_plane.foreach_surface(region_lambda)
	
func _region_first_pass(region: RegionObject, region_qr_coords: Vector2i) ->void:		
	var region_cell_lambda := func(cell_q: int, cell_r: int) ->void:
		_cell_first_pass(region, Vector2i(cell_q, cell_r))
		
	region.foreach(region_cell_lambda)

	
func _cell_first_pass(region: RegionObject, cell_qr_coords: Vector2i) -> void:
	var temperature : float = region.get_temperature(cell_qr_coords)
	var precipitation : float = region.get_precipitation(cell_qr_coords)
	var height : float = region.get_height(cell_qr_coords)
	
	var biome : StringName
	if height >= 0:
		biome = _get_biome(int(temperature), int(precipitation))
	else:
		biome = &"core.biome.ocean"
		
	region.set_biome(cell_qr_coords, biome)
	

func _get_biome(temperature: int, precipation: int) -> StringName:
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
