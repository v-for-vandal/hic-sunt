extends RefCounted

var _ui = preload("res://content/worldgen-basic/worldgen/ui/heightmap.tscn")

func create_heightmap(plane: PlaneObject, config: Variant):
	pass
	
func create_heightmap_ui() -> Control:
	return _ui.new()
	
