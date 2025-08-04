extends RefCounted

var _ui := preload("res://content/worldgen-basic/worldgen/ui/heightmap.tscn")
var _heightmap_generator := preload("res://content/worldgen-basic/worldgen/internal/heightmap_generator.gd")

func get_module_info() -> Dictionary:
	return {}

func create_heightmap_generator(plane: PlaneObject, config: Variant, global_context: Dictionary[StringName, Variant]) -> WorldGeneratorModuleInterface:
	return _heightmap_generator.new(plane, config, global_context)
	
func create_heightmap_generator_ui() -> Control:
	return _ui.instantiate()
	
