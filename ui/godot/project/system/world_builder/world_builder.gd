extends Node

@export var terrain_noise_generator : FastNoiseLite
@export var mountain_noise_generator: FastNoiseLite
@export var temperature_noise_generator: FastNoiseLite
@export var percipation_noise_generator: FastNoiseLite
@export var temperature_curve : Curve
@export var debug_mode := false

# public signals
signal emit_debug_map(name: String, map: Image, legend: Dictionary)


# private constants

const _TEMPERATURE_RANGE = Vector2i(-20, 20)
const _TEMPERATURE_FLUCTUATION = 5

const _HEIGHT_RANGE = Vector2i(-8000, 8000) # meters
const _MOUNTAIN_EXTRA_HEIGHT_RANGE = Vector2i(3000, 6000) # meters

const MIN_REGION_SIZE = 10
const INTER_REGION_MARGIN = 10

var _DEBUG_GENERAL_GRADIENT : Gradient = ResourceLoader.load("res://system/world_builder/debug_genral_gradientt.tres")
var _DEBUG_HEIGHT_GRADIENT : Gradient = ResourceLoader.load("res://system/world_builder/debug_height_gradient.tres")
# private variables

# Size of the map (image, texutere) that we should generate
var _generation_map_size := Rect2i( Vector2i(0,0), Vector2i(100, 100))

var _heightmap : Image



func make_biome_rect(temp_start:int, temp_end:int, percipation_start:int, percipation_end:int) -> Rect2i:
	return Rect2i( temp_start, percipation_start, (temp_end-temp_start), (percipation_end - percipation_start))
	
# TODO: Move to proto
var biome_maps = [
	{
		make_biome_rect(-10, 0, 0, 400) : "core.terrain.tundra",
		make_biome_rect(0, 5, 0, 20) : "core.terrain.temperate_grassland",
		make_biome_rect(5, 20, 0, 40) : "core.terrain.temperate_grassland",
		make_biome_rect(0, 5, 20, 400 ) : "core.terrain.taiga",
		make_biome_rect(20, 30, 0, 100 ) : "core.terrain.desert",
		make_biome_rect(5, 15, 40, 200) : "core.terrain.temperate_forest",
		make_biome_rect(15, 20, 120, 230) : "core.terrain.temperate_forest",
	}
]

var terrain_colors = {
	"core.terrain.tundra" : Color.FLORAL_WHITE,
	"core.terrain.temperate_grassland" : Color.DARK_OLIVE_GREEN,
	"core.terrain.taiga" : Color.BEIGE,
	"core.terrain.desert" : Color.GOLDENROD,
	"core.terrain.temperate_forest" : Color.DARK_GREEN
}

# assumes that value is between -1 and 1.
func _sample_simple_range(value: float, range : Vector2i) -> int:
	return range.x + (range.y - range.x) * (value - (-1.0)) / 2
	
func _displacement_at_range(value: float, range: Vector2i) -> float:
	return (value - range.x) / (range.y - range.x)

func _get_biome(temperature: int, precipation: int) -> String:
	var point := Vector2i(temperature, precipation)
	
	for biome_map : Dictionary in biome_maps:
		for rect : Rect2i in biome_map:
			if rect.has_point(point):
				return biome_map[rect]
	return "core.terrain.unknown"
	
func _get_height_at_point(i: int, j : int) -> int:
	# Get base height. It will be in range (-1, 1)
	var height_float = terrain_noise_generator.get_noise_2d(i, j)
	
	var height_meters = _sample_simple_range(height_float, _HEIGHT_RANGE)
	
	# add mountains
	var mountain_extra_float = mountain_noise_generator.get_noise_2d(i,j)
	var mountain_extra_meters = 0 # by default, don't add anything
	if mountain_extra_float > 0.75:
		# renormalize to range (-1,1) because that is what _sample_simple_range
		# expects
		mountain_extra_float =  -1 + ( mountain_extra_float - 0.75) / (1 - 0.75) * 2
		mountain_extra_meters = _sample_simple_range(mountain_extra_float, _MOUNTAIN_EXTRA_HEIGHT_RANGE)
		
	var result := int(clamp(height_meters + mountain_extra_meters, _HEIGHT_RANGE.x, _HEIGHT_RANGE.y))
	
	return result
	
	
	
func _create_debug_height_map() -> void:
	var size = _generation_map_size.size
	var height_image = Image.create(size.x, size.y, false, Image.FORMAT_RGB8)
	for i in range(0, size.x):
		for j in range(0, size.y):
			var height := _get_height_at_point(i,j)
			var color := _DEBUG_HEIGHT_GRADIENT.sample(
				_displacement_at_range(height, _HEIGHT_RANGE)
			)
			height_image.set_pixel(i, j, color)
	emit_debug_map.emit("heightmap", height_image, {})

# Terrain map is like height map, but with less colors for clarity.
# Good for backgrounds
func _create_debug_terrain_map() -> void:
	var size = _generation_map_size.size
	var terrain_image = Image.create(size.x, size.y, false, Image.FORMAT_RGB8)

	for i in range(0, size.x):
		for j in range(0, size.y):
			var value := _get_height_at_point(i,j)
			if value < 0:
				terrain_image.set_pixel(i, j, Color.DARK_BLUE)
			elif value < 4000:
				# normal(ish) land
				terrain_image.set_pixel(i,j, Color.CORAL)
			else:
				terrain_image.set_pixel(i, j, Color.LIGHT_GRAY)

		emit_debug_map.emit("terrain", terrain_image, {})

func _get_temperature_at_point(i, j) -> int:
	# base temperature = how close is point to the borders
	var j_percent : float  = float(j - _generation_map_size.position.y) /  _generation_map_size.size.y
	var point_on_curve : float = temperature_curve.sample(j_percent)
	var temperature : int = _sample_simple_range(point_on_curve, _TEMPERATURE_RANGE)
	
	if i % 10 == 0 and j % 10 == 0:
		print("Temperature at ", i, ",", j, ": ", j_percent, ", ", point_on_curve, ", ", temperature)

	# add fluctuation from the map
	var temp_fluctuation : int = int(terrain_noise_generator.get_noise_2d(i,j) * _TEMPERATURE_FLUCTUATION)
	temperature += temp_fluctuation
	
	temperature = int(clamp(temperature, _TEMPERATURE_RANGE.x, _TEMPERATURE_RANGE.y))
	return temperature

# Create temperature map and emits it for debugging
func _create_debug_temperature_map():
	var size = _generation_map_size.size
	var temperature_image = Image.create(size.x, size.y, false, Image.FORMAT_RGB8)
	for i in range(0, size.x):
		for j in range(0, size.y):
			var temp := _get_temperature_at_point(i,j)
			var color = _DEBUG_GENERAL_GRADIENT.sample(
				_displacement_at_range(temp, _TEMPERATURE_RANGE)
			)
			temperature_image.set_pixel(i, j, color)
			
	emit_debug_map.emit("temperature", temperature_image, {})
	
func _get_biome_at_point(i: int, j:int) -> String:
	return ""
	


func _generate_biome_map() -> void:
	_create_debug_height_map()
	_create_debug_temperature_map()
	_create_debug_terrain_map()
		
func generate(world_cells_size: Vector2i, region_size: int) -> void:
	if region_size < MIN_REGION_SIZE:
		region_size = MIN_REGION_SIZE
		
	# first, we determine required size of generated image. It doesn't have to be
	# precise, it just have to be enough
	# generated image size is 
	# x - number of cells on x axis * ( region_size (region_size is diameter) + margin)
	# y - ( number of cells on y axis / 2  rounded up) * ( 2 * sqrt(3) * region_size + margin)
	#var generated_map_size = Vector2i(
	#	world_cells_size.x * (region_size + INTER_REGION_MARGIN),
	#	(world_cells_size.y / 2 + (world_cells_size.y & 1)) * (2 * 1.75 * region_size + margin)
	#)
	
	_generation_map_size = Rect2i(Vector2i.ZERO, Vector2i(800, 800))

	
	# Now that we have map size, lets generate biome map
	_generate_biome_map()
	
	# Now, lets build hexagon map via this generated map
	

func _on_world_builder_generate_requested(world_size: WBConstants.WorldSize, region_size: int) -> void:
	var world_cells_size = Vector2i(20, 20)
	if(world_size == WBConstants.WorldSize.Normal):
		world_cells_size = Vector2i(40, 40)
		
	generate(world_cells_size, region_size)
