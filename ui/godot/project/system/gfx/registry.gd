extends Node

var _biomes: Dictionary[StringName, Image]
const  _BIOME_IMAGE_SIZE := Vector2i(512, 512)
var _unknown_biome : Image = _load_unknown_biome_image()

static func _load_unknown_biome_image() -> Image:
	var result = load("res://resources/pink512x512.png") as Image
	result.resize(_BIOME_IMAGE_SIZE.x, _BIOME_IMAGE_SIZE.y, Image.INTERPOLATE_NEAREST)
	return result

func register_biome_texture(biome : StringName, image : Image) -> void:
	image.resize(_BIOME_IMAGE_SIZE.x, _BIOME_IMAGE_SIZE.y, Image.INTERPOLATE_CUBIC)
	_biomes[biome] = image
	
func get_biome_texture(biome: StringName) -> Image:
	return _biomes.get(biome, _unknown_biome)
	
	
