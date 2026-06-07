extends Node

var _biomes_images: Dictionary[StringName, Image]
var _biomes_textures : Dictionary[StringName, Texture2D]
var _improvements_images: Dictionary[StringName, Image]
var _improvements_textures : Dictionary[StringName, Texture2D]
const  _BIOME_IMAGE_SIZE := Vector2i(512, 512)
const  _IMPROVEMENT_IMAGE_SIZE := Vector2i(512, 512)
const _FALLBACK_IMPROVEMENT_IMAGE_NAME := &"core.building.unknown"
var _unknown_biome : Image = _load_unknown_biome_image()
var _unknown_biome_texture : ImageTexture = _load_unknown_biome_texture()
var _unknown_improvement : Image = _load_unknown_improvement_image()
var _unknown_improvement_texture : ImageTexture = _load_unknown_improvement_texture()

static func _load_unknown_biome_image() -> Image:
	var result := load("res://resources/pink512x512.png") as Image
	result.resize(_BIOME_IMAGE_SIZE.x, _BIOME_IMAGE_SIZE.y, Image.INTERPOLATE_NEAREST)
	return result

static func _load_unknown_biome_texture() -> ImageTexture:
	return ImageTexture.create_from_image(_load_unknown_biome_image())


static func _load_unknown_improvement_image() -> Image:
	var result := load("res://resources/pink512x512.png") as Image
	result.resize(_IMPROVEMENT_IMAGE_SIZE.x, _IMPROVEMENT_IMAGE_SIZE.y, Image.INTERPOLATE_NEAREST)
	return result

static func _load_unknown_improvement_texture() -> ImageTexture:
	return ImageTexture.create_from_image(_load_unknown_improvement_image())


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


func register_improvement_image(improvement : StringName, image : Image) -> void:
	print("Registering image for improvement: \"%s\"" %[improvement])
	image.resize(_IMPROVEMENT_IMAGE_SIZE.x, _IMPROVEMENT_IMAGE_SIZE.y, Image.INTERPOLATE_CUBIC)
	if image.resource_name == "":
		image.resource_name =  image.resource_path

	_improvements_images[improvement] = image
	_improvements_textures[improvement] = ImageTexture.create_from_image(image)
	_improvements_textures[improvement].resource_name = image.resource_name

func get_improvement_image(improvement: StringName) -> Image:
	if not (improvement  in _improvements_images):
		print("No image provide for improvement\"%s\" " % [improvement])
		
	var fallback_image : Image = _improvements_images.get(_FALLBACK_IMPROVEMENT_IMAGE_NAME, _unknown_improvement)

	return _improvements_images.get(improvement, fallback_image)

func get_improvement_texture(improvement: StringName) -> Texture2D:
	if not (improvement  in _improvements_textures):
		print("No texture provide for improvement \"%s\"" % [improvement])
		
	var fallback_texture : Texture2D = _improvements_images.get(_FALLBACK_IMPROVEMENT_IMAGE_NAME, _unknown_improvement_texture)


	return _improvements_textures.get(improvement, fallback_texture)
