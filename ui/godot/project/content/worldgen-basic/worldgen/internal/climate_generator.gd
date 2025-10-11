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
# North/Source pole effect gradually decreases over this number of cells
var _pole_effect_distance: = 7.0
# This is total region radius, with margin
var _region_radius : int = 1


func _init(plane: PlaneObject, config: Variant, global_context: Dictionary[StringName, Variant]) -> void:
	super(global_context)
	_config = config
	_plane = plane
	_global_context = global_context
	
	_region_radius = _global_context[&"region.radius"] + RADIUS_MARGIN
	# that is definetly wrong, but at the moment it will suffice
	var world_bbox : Rect2i = _global_context[&"world.bbox"]

	
func first_pass() -> void:
	
	var region_lambda := func(region_q: int, region_r: int, region: RegionObject) -> void:
		_region_first_pass(region, Vector2i(region_q, region_r))
		
	_plane.foreach_surface(region_lambda)
	
func _region_first_pass(region: RegionObject, region_qrs_coords: Vector2i) ->void:
	var radius : int = _global_context[&"region.radius"]
	
	var precipitation := _get_precipation_at_point(region_qrs_coords, Vector2i.ZERO)
	
	_set_baseline_temperature(region, region_qrs_coords)
	region.get_scope().add_numeric_modifier(Modifiers.ECOSYSTEM_PRECIPITATION, &"wordlgen.basic", precipitation, 0.0)

	
	var region_cell_lambda := func(cell_q: int, cell_r: int) ->void:
		_cell_first_pass(region, region_qrs_coords, Vector2i(cell_q, cell_r))
		
	region.foreach(region_cell_lambda)

	
func _cell_first_pass(region: RegionObject, region_coords: Vector2i, cell_qr_coords: Vector2i) -> void:
	var cell : CellObject = region.get_cell(cell_qr_coords)
	var height : float = cell.get_scope().get_numeric_value(Modifiers.GEOGRAPHY_HEIGHT)
	# temperature decreases at rate 6.5 celsius / 1000m
	var temperature_modifier := (-6.5 / 1000.0) * height
	# We can do some fine tuning here
	cell.get_scope().add_numeric_modifier(Modifiers.ECOSYSTEM_TEMPERATURE, &"worldgen.lapse_rate", temperature_modifier, 0.0)
	
	# add some random fluctuation
	var total_coords := region_coords * _region_radius + cell_qr_coords
	var temp_fluctuation : float = _temperature_noise.get_noise_2d(total_coords.x,total_coords.y) * _TEMPERATURE_FLUCTUATION
	cell.get_scope().add_numeric_modifier(Modifiers.ECOSYSTEM_TEMPERATURE, &"wordgeо.random_fluctuation", temp_fluctuation, 0.0)

	
func _distance_from_poles(region_coords: Vector2i, cell_qr_coords: Vector2i) -> float:
	var distance := _plane.get_distance_between_cells(region_coords, cell_qr_coords,
	_pole_1, Vector2i(0,0))
	return distance

func _set_baseline_temperature(region: RegionObject, region_coords: Vector2i) -> void:
	var distance_from_poles := _distance_from_poles(region_coords, Vector2i.ZERO) 
	var j_percent := distance_from_poles / _pole_effect_distance
	j_percent = clampf(j_percent, 0.0, 1.0)
	var point_on_curve : float = _temperature_curve.sample(j_percent)
	# sample_simple_range_i expects value between -1.0, 1.0
	assert(point_on_curve >= -1.0 && point_on_curve <= 1.0)
	var temperature : int = NoiseToolsLibrary.sample_simple_range_i(point_on_curve, _TEMPERATURE_RANGE)
	region.get_scope().add_numeric_modifier(Modifiers.ECOSYSTEM_TEMPERATURE, &"worldgen.basic.dist_to_poles", temperature, 0.0)
	# region.get_scope().add_numeric_modifier(&"debug.dist_to_poles_percent", &"worldgen.basic", j_percent, 0.0)
	# region.get_scope().add_numeric_modifier(&"debug.temp_point_on_curve", &"worldgen.basic", point_on_curve, 0.0)
	# region.get_scope().add_numeric_modifier(&"debug.distance_to_pole", &"worldgen.basic", distance_from_poles, 0.0)


func _get_temperature_at_point(region_coords: Vector2i, cell_qr_coords: Vector2i) -> int:
	var total_coords := region_coords * _region_radius + cell_qr_coords

	# base temperature = how close is point to the north/south poles
	var distance_from_poles := _distance_from_poles(region_coords, cell_qr_coords) 
	var j_percent := distance_from_poles / _pole_effect_distance
	j_percent = clampf(j_percent, 0.0, 1.0)
	var point_on_curve : float = _temperature_curve.sample(j_percent)
	# sample_simple_range_i expects value between -1.0, 1.0
	assert(point_on_curve >= -1.0 && point_on_curve <= 1.0)
	var temperature : int = NoiseToolsLibrary.sample_simple_range_i(point_on_curve, _TEMPERATURE_RANGE)

	# add fluctuation from the map
	var temp_fluctuation : int = int(_temperature_noise.get_noise_2d(total_coords.x,total_coords.y) * _TEMPERATURE_FLUCTUATION)
	temperature += temp_fluctuation
	
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
	
func _get_precipation_at_point(region_coords: Vector2i, cell_qr_coords: Vector2i) -> int:
	var total_coords := region_coords * _region_radius + cell_qr_coords
	# Get base height. It will be in range (-1, 1)
	var precipation_float := _precipitation_noise.get_noise_2d(total_coords.x, total_coords.y)
	
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
