extends WorldBuilderGeneratorInterface

# user-selected options
const Config = preload("res://content/worldgen-basic/worldgen/internal/heightmap_gen_config.gd")

var config : Config
var plane: PlaneObject
var terrain_noise_generator_ : FastNoiseLite = preload(
	"res://content/worldgen-basic/worldgen/internal/heightmap_noise_base.tres").duplicate()
var mountain_noise_generator_: FastNoiseLite = preload(
	"res://content/worldgen-basic/worldgen/internal/heightmap_noise_mountain.tres").duplicate()


func _init(plane_: PlaneObject, config_: Variant):
	config = config_
	plane = plane_
	
func first_pass(global_context: Dictionary[StringName, Variant],
	generator_context: Dictionary[StringName, Variant]):
	pass
	
func _wrap_x(i: float) -> float:
	# Wrapping will be done within plane object itself
	return i
	
	
func _wrap_y(j: float) -> float:
	return j

# TODO: Make some other formula
var _MOUNTAIN_EXTRA_HEIGHT_RANGE := Vector2i(1000, 2000)

func _displacement_at_range(value: float, range: Vector2i) -> float:
	return (value - range.x) / (range.y - range.x)

func _get_height_at_point(i: float, j : float) -> int:
	i = _wrap_x(i)
	j = _wrap_y(j)
	# Get base height. It will be in range (-1, 1)
	var sample_value := terrain_noise_generator_.get_noise_2d(i, j)
	
	var height_meters := NoiseToolsLibrary.sample_simple_range_i(sample_value, config.height_range)
	
	# add mountains
	var mountain_sample_value := mountain_noise_generator_.get_noise_2d(i,j)
	var mountain_extra_meters = 0 # by default, don't add anything
	if mountain_sample_value > 0.75:
		# renormalize to range (-1,1) because that is what _sample_simple_range
		# expects
		mountain_sample_value =  -1 + ( mountain_sample_value - 0.75) / (1 - 0.75) * 2
		mountain_sample_value = NoiseToolsLibrary.sample_simple_range_i(mountain_sample_value, _MOUNTAIN_EXTRA_HEIGHT_RANGE)
		
	var result := int(clamp(height_meters + mountain_extra_meters, config.height_range.x, config.height_range.y))
	
	return result
