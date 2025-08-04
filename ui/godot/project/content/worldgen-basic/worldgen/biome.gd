extends RefCounted

# No UI at the moment
#var _ui := preload("res://content/worldgen-basic/worldgen/ui/biome.tscn")
var _biome_generator := preload("res://content/worldgen-basic/worldgen/internal/biome_generator.gd")

func get_module_info() -> Dictionary:
	return {}

func create_biome_generator(plane: PlaneObject, config: Variant, global_context: Dictionary[StringName, Variant]) -> WorldGeneratorModuleInterface:
	return _biome_generator.new(plane, config, global_context)
	
func create_biome_generator_ui() -> Control:
	return null
	
