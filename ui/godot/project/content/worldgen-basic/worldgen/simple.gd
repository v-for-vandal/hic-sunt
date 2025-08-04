extends RefCounted

var _ui := preload("res://content/worldgen-basic/worldgen/ui/main.tscn")
var _world_generator := preload("res://content/worldgen-basic/worldgen/internal/world_generator.gd")

func get_module_info() -> Dictionary[StringName, Variant]:
	return {
		&'name' : 'SimpleWorldGenerator'
	}

func create_world_generator(config: Variant) -> WorldGeneratorInterface:
	return _world_generator.new(config)
	
func create_world_generator_ui() -> Control:
	return _ui.instantiate()
	
