extends GridContainer


const Config = preload("res://content/worldgen-basic/worldgen/internal/heightmap_gen_config.gd")


# Called when the node enters the scene tree for the first time.
func get_config() -> Variant:
	var result := Config.new()
	result.height_range.x = $MinHeightSlider.value * 1000 # convert km to m
	result.height_range.y = $MaxHeightSlider.value * 1000 # convert km to m
	
	return  result
