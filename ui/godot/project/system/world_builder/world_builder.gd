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

const MIN_REGION_SIZE = 10
const INTER_REGION_MARGIN = 10

var _DEBUG_GENERAL_GRADIENT : Gradient = ResourceLoader.load("res://system/world_builder/debug_genral_gradientt.tres")

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

func _get_biome(temperature: int, precipation: int) -> String:
	var point := Vector2i(temperature, precipation)
	
	for biome_map : Dictionary in biome_maps:
		for rect : Rect2i in biome_map:
			if rect.has_point(point):
				return biome_map[rect]
	return "core.terrain.unknown"
	
func _generate_terrain_map() -> void:
	var size = _generation_map_size.size
	#var heightmap := Image.create(size.x, size.y, false, Image.FORMAT_R8)
	
	#for i in range(0, size.x):
	#	for j in range(0, size.y):
	
	var heightmap := terrain_noise_generator.get_image(size.x, size.y)
	var terrain_image = Image.create(size.x, size.y, false, Image.FORMAT_RGB8)
	
	
	
	for i in range(0, size.x):
		for j in range(0, size.y):
			var value := terrain_noise_generator.get_noise_2d(i,j)
			if value < 0:
				terrain_image.set_pixel(i, j, Color.DARK_BLUE)
			else:
				if mountain_noise_generator.get_noise_2d(i,j) > 0.7:
					terrain_image.set_pixel(i, j, Color.LIGHT_GRAY)
				else:
					terrain_image.set_pixel(i,j, Color.CORAL)
	if debug_mode:
		emit_debug_map.emit("terrain", terrain_image, {})

func _get_temperature_at_point(i, j) -> int:
	# base temperature = how close is point to the borders
	var j_percent : float  = j /  _generation_map_size.size.y
	var point_on_curve : float = temperature_curve.sample(j_percent)
	var temperature : int = _TEMPERATURE_RANGE.x + int(
		(_TEMPERATURE_RANGE.y - _TEMPERATURE_RANGE.x) * point_on_curve)
		
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
				(temp - _TEMPERATURE_RANGE.x) / (_TEMPERATURE_RANGE.y - _TEMPERATURE_RANGE.x)
			)
			temperature_image.set_pixel(i, j, color)
			
	emit_debug_map.emit("temperature", temperature_image, {})
	
func _get_biome_at_point(i: int, j:int) -> String:
	return ""
	


func _generate_biome_map() -> void:
	_generate_terrain_map()
		
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
	
	_generation_map_size = Rect2i(Vector2i.ZERO, Vector2i(1000, 1000))

	
	# Now that we have map size, lets generate biome map
	_generate_biome_map()
	
	# Now, lets build hexagon map via this generated map
	

func _on_world_builder_generate_requested(world_size: WBConstants.WorldSize, region_size: int) -> void:
	var world_cells_size = Vector2i(20, 20)
	if(world_size == WBConstants.WorldSize.Normal):
		world_cells_size = Vector2i(40, 40)
		
	generate(world_cells_size, region_size)
