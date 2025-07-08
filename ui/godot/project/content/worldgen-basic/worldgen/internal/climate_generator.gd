extends WorldGeneratorModuleInterface

# user-selected options
const Config = preload("res://content/worldgen-basic/worldgen/internal/climate_gen_config.gd")

var _temperature_curve : Curve = preload("res://content/worldgen-basic/worldgen/internal/temperature_curve.tres")
var _temperature_noise : FastNoiseLite = preload("res://content/worldgen-basic/worldgen/internal/temperature_noise_fluct.tres")
var _precipitation_noise : FastNoiseLite = preload("res://content/worldgen-basic/worldgen/internal/precipitation_noise.tres")

const _TEMPERATURE_RANGE = Vector2i(-20, 40) # celsius
const _TEMPERATURE_FLUCTUATION = 5

const _PRECIPATION_RANGE = Vector2i(0, 400)

const RADIUS_MARGIN : int = 2


var _config : Config
var _plane: PlaneObject
var _global_context : Dictionary[StringName, Variant]

# extracted from _global_context
# This is actually incorrect math, but at the moment we don't care
var _pole_1 := Vector2i(0, 0)
# This is total region radius, with margin
var _region_radius : int = 1

var _world_size_squared : int = 1


func _init(plane: PlaneObject, config: Variant, global_context: Dictionary[StringName, Variant]) -> void:
	super(global_context)
	_config = config
	_plane = plane
	_global_context = global_context
	
	_region_radius = _global_context[&"region.radius"] + RADIUS_MARGIN
	# that is definetly wrong, but at the moment it will suffice
	var world_bbox : Rect2i = _global_context[&"world.bbox"]
	_world_size_squared = QrsCoordsLibrary.distance_squared_qr(world_bbox.position, world_bbox.end)
	
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

	
func _cell_first_pass(region: RegionObject, region_coords: Vector2i, cell_qr_coords: Vector2i) -> void:
	var temperature := _get_temperature_at_point(region_coords, cell_qr_coords)
	region.set_temperature(cell_qr_coords, temperature)
	
func _distance_from_poles_squared(region_coords: Vector2i, cell_qr_coords: Vector2i) -> int:
	var total_coords = region_coords * _region_radius + cell_qr_coords
	return QrsCoordsLibrary.distance_squared_qr(_pole_1, total_coords)

	
func _get_temperature_at_point(region_coords: Vector2i, cell_qr_coords: Vector2i) -> int:
	var total_coords = region_coords * _region_radius + cell_qr_coords

	# base temperature = how close is point to the borders
	var j_percent : float  = float(_distance_from_poles_squared(region_coords, cell_qr_coords))  /  _world_size_squared
	var point_on_curve : float = _temperature_curve.sample(j_percent)
	var temperature : int = NoiseToolsLibrary.sample_simple_range_i(point_on_curve, _TEMPERATURE_RANGE)
	
	#if i % 10 == 0 and j % 10 == 0:
	#	print("Temperature at ", i, ",", j, ": ", j_percent, ", ", point_on_curve, ", ", temperature)

	# add fluctuation from the map
	var temp_fluctuation : int = int(_temperature_noise.get_noise_2d(total_coords.x,total_coords.y) * _TEMPERATURE_FLUCTUATION)
	#temperature += temp_fluctuation
	
	temperature = int(clamp(temperature, _TEMPERATURE_RANGE.x, _TEMPERATURE_RANGE.y))
	return temperature

# Create temperature map and emits it for debugging
#func _create_debug_temperature_map() -> void:
	#if not debug_mode:
		#return
	#var size = _generation_map_size.size
	#var temperature_image = Image.create(size.x, size.y, false, Image.FORMAT_RGB8)
	#for i in range(0, size.x):
		#for j in range(0, size.y):
			#var temp := _get_temperature_at_point(i,j)
			#var color = _DEBUG_GENERAL_GRADIENT.sample(
				#_displacement_at_range(temp, _TEMPERATURE_RANGE)
			#)
			#temperature_image.set_pixel(i, j, color)
			#
	#_emit_debug_map("temperature", temperature_image, {})
	
func _get_precipation_at_point(i : float, j : float) -> int:
	# Get base height. It will be in range (-1, 1)
	var precipation_float = _precipitation_noise.get_noise_2d(i, j)
	
	var precipation_cm := NoiseToolsLibrary.sample_simple_range_i(precipation_float, _PRECIPATION_RANGE)
	return precipation_cm
	
#func _create_debug_precipation_map() -> void:
	#if not debug_mode:
		#return
	#var size = _generation_map_size.size
	#var precipation_image = Image.create(size.x, size.y, false, Image.FORMAT_RGB8)
	#for i in range(0, size.x):
		#for j in range(0, size.y):
			#var precip := _get_precipation_at_point(i,j)
			#var color = _DEBUG_GENERAL_GRADIENT.sample(
				#_displacement_at_range(precip, _PRECIPATION_RANGE)
			#)
			#precipation_image.set_pixel(i, j, color)
			#
	#_emit_debug_map("precipation", precipation_image, {})
