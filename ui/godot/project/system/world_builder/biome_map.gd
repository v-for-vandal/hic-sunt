extends Resource
class_name BiomeMap

var _biome_map : Array= []


func get_biome(temperature: int, precipation: int) -> String:
	return "core.biome.unknown"
	
