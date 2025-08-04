extends WorldGeneratorInterface

const Config = preload("res://content/worldgen-basic/worldgen/internal/world_generator_config.gd")

var _config: Config

func _init(config : Config) -> void:
	_config = config

func create_world() -> WorldObject:
	var world := WorldObject.new()
	
	var global_context : Dictionary[StringName, Variant] = {}
	
	# add plane
	# For now, size is fixed
	var world_size := Rect2i(Vector2i(0,0), Vector2i(0, 0))
	var region_radius : int = 3
	
	var main_plane : PlaneObject = world.create_plane(&"main", world_size, region_radius, -1)
	
	global_context[&"world.bbox"] = world_size
	global_context[&"region.radius"] = region_radius
	global_context[&"seed"] = _config.seed
	
	# Call height generator
	var heightmap_generator := _config.heightmap_module.create_generator(
		main_plane, _config.heightmap_config, global_context
	)
	heightmap_generator.first_pass()
	
	# Call climate generator
	var climate_generator := _config.climate_module.create_generator(
		main_plane, _config.climate_config, global_context
	)
	climate_generator.first_pass()
	
	# Call climate generator
	var biome_generator := _config.biome_module.create_generator(
		main_plane, _config.biome_config, global_context
	)
	biome_generator.first_pass()
	
	
	return world
