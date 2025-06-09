extends RefCounted

var _ui = preload("res://content/worldgen-basic/worldgen/ui/heightmap.tscn")
var _heightmap_generator = preload("res://content/worldgen-basic/worldgen/internal/heightmap_generator.gd")

func create_heightmap_generator(plane: PlaneObject, config: Variant) -> WorldBuilderGeneratorInterface:
	return _heightmap_generator.new(plane, config)
	
func create_heightmap_ui() -> Control:
	return _ui.new()
	
