extends VBoxContainer

const Config = preload("res://content/worldgen-basic/worldgen/internal/world_generator_config.gd")


# Called when the node enters the scene tree for the first time.
func get_config() -> Variant:
	var result := Config.new()

	result.heightmap_module = $HeightmapSelector.get_selected_module()
	result.heightmap_config = $HeightmapSelector.get_selected_config()

	result.climate_module = $ClimateSelector.get_selected_module()
	result.climate_config = $ClimateSelector.get_selected_config()

	result.biome_module = $BiomeSelector.get_selected_module()
	result.biome_config = $BiomeSelector.get_selected_config()

	return result
