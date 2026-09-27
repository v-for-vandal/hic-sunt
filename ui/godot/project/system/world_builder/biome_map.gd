extends Resource

class_name BiomeMap

var _biome_map: Array = []


func get_biome(_temperature: int, _precipation: int) -> String:
	return "core.biome.unknown"
