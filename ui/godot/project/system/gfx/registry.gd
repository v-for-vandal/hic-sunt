extends Node

var _biomes_images: Dictionary[StringName, Image]
var _biomes_textures : Dictionary[StringName, Texture2D]
const  _BIOME_IMAGE_SIZE := Vector2i(512, 512)
var _unknown_biome : Image = _load_unknown_biome_image()
var _unknown_biome_texture : ImageTexture = _load_unknown_biome_texture()

static func _load_unknown_biome_image() -> Image:
	var result := load("res://resources/pink512x512.png") as Image
	result.resize(_BIOME_IMAGE_SIZE.x, _BIOME_IMAGE_SIZE.y, Image.INTERPOLATE_NEAREST)
	return result
	
static func _load_unknown_biome_texture() -> ImageTexture:
	return ImageTexture.create_from_image(_load_unknown_biome_image())


func register_biome_image(biome : StringName, image : Image) -> void:
	print("Registering image for biome: \"%s\"" %[biome])
	image.resize(_BIOME_IMAGE_SIZE.x, _BIOME_IMAGE_SIZE.y, Image.INTERPOLATE_CUBIC)
	if image.resource_name == "":
		image.resource_name =  image.resource_path

	_biomes_images[biome] = image
	_biomes_textures[biome] = ImageTexture.create_from_image(image)
	_biomes_textures[biome].resource_name = image.resource_name
	
func get_biome_image(biome: StringName) -> Image:
	if not (biome  in _biomes_images):
		print("No image provide for biome\"%s\" " % [biome])

	return _biomes_images.get(biome, _unknown_biome)
	
func get_biome_texture(biome: StringName) -> Texture2D:
	if not (biome  in _biomes_textures):
		print("No texture provide for biome \"%s\"" % [biome])

	return _biomes_textures.get(biome, _unknown_biome_texture)
	
	
