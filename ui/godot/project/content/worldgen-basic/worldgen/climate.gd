extends RefCounted

# no UI here for now
#var _ui := preload("res://content/worldgen-basic/worldgen/ui/climate.tscn")
var _climate_generator := preload("res://content/worldgen-basic/worldgen/internal/climate_generator.gd")

func get_module_info() -> Dictionary:
	return {}

func create_climate_generator(plane: PlaneObject, config: Variant, global_context: Dictionary[StringName, Variant]) -> WorldGeneratorModuleInterface:
	return _climate_generator.new(plane, config, global_context)
	
func create_climate_generator_ui() -> Control:
	return null
	
