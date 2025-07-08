extends WorldGeneratorModuleInterface

# user-selected options
const Config = preload("res://content/worldgen-basic/worldgen/internal/heightmap_gen_config.gd")

var _config : Config
var _plane: PlaneObject
var _global_context : Dictionary[StringName, Variant]
var _terrain_noise_generator : FastNoiseLite = preload(
	"res://content/worldgen-basic/worldgen/internal/heightmap_noise_base.tres").duplicate()
var _mountain_noise_generator: FastNoiseLite = preload(
	"res://content/worldgen-basic/worldgen/internal/heightmap_noise_mountain.tres").duplicate()
	
const RADIUS_MARGIN : int = 2


func _init(plane: PlaneObject, config: Variant, global_context: Dictionary[StringName, Variant]) -> void:
	super(global_context)
	_config = config
	_plane = plane
	_global_context = global_context
	
	
func first_pass() -> void:
	
	var region_lambda= func(region_q: int, region_r: int, region: RegionObject) -> void:
		_region_first_pass(region, Vector2i(region_q, region_r))
		
	_plane.foreach_surface(region_lambda)
	
func _region_first_pass(region: RegionObject, region_qrs_coords: Vector2i) ->void:
	var radius : int = _global_context[&"region.radius"]
	var region_coords := Vector2i(
		 region_qrs_coords.x * (radius + RADIUS_MARGIN),
		region_qrs_coords.y * (radius + RADIUS_MARGIN))

	
	
	var region_cell_lambda = func(cell_q: int, cell_r: int) ->void:
		_cell_first_pass(region, region_coords, Vector2i(cell_q, cell_r))
		
	region.foreach(region_cell_lambda)

	
func _cell_first_pass(region: RegionObject, region_coords: Vector2i, cell_qrs_coords: Vector2i) -> void:
	var height := _get_height_at_point(region_coords.x + cell_qrs_coords.x, region_coords.y + cell_qrs_coords.y)
	region.set_height(cell_qrs_coords, height)
	
	
func _wrap_x(i: float) -> float:
	# Wrapping will be done within plane object itself
	return i
	
	
func _wrap_y(j: float) -> float:
	return j

# TODO: Make some other formula
var _MOUNTAIN_EXTRA_HEIGHT_RANGE := Vector2i(1000, 2000)

func _displacement_at_range(value: float, target_range: Vector2i) -> float:
	return (value - target_range.x) / (target_range.y - target_range.x)

func _get_height_at_point(i: float, j : float) -> int:
	i = _wrap_x(i)
	j = _wrap_y(j)
	# Get base height. It will be in range (-1, 1)
	var sample_value := _terrain_noise_generator.get_noise_2d(i, j)
	
	var height_meters := NoiseToolsLibrary.sample_simple_range_i(sample_value, _config.height_range)
	
	# add mountains
	var mountain_sample_value := _mountain_noise_generator.get_noise_2d(i,j)
	var mountain_extra_meters := 0 # by default, don't add anything
	if mountain_sample_value > 0.75:
		# renormalize to range (-1,1) because that is what _sample_simple_range
		# expects
		mountain_sample_value =  -1 + ( mountain_sample_value - 0.75) / (1 - 0.75) * 2
		mountain_sample_value = NoiseToolsLibrary.sample_simple_range_i(mountain_sample_value, _MOUNTAIN_EXTRA_HEIGHT_RANGE)
		
	var result := int(clamp(height_meters + mountain_extra_meters, _config.height_range.x, _config.height_range.y))
	
	return result
