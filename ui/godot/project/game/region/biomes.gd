extends Sprite2D

var _biomes : Image
var _biomes_atlas : Texture2DArray = Texture2DArray.new()
var _biomes_mapping : Dictionary[StringName, int]

# textures always start from (0,0), so we keep origin point.
var _offset_dimensions: Rect2i

# as per https://www.redblobgames.com/grids/hexagons/#basics
const CELL_SIZE : int = 64

func _init() -> void:
	_biomes = Image.create_empty(10, 10, false, Image.Format.FORMAT_RGF)

func clear():
	_biomes_mapping.clear()
	_biomes.fill(Color(0.0, 0.0, 0.0, 0.0))

# dimensions are in axial coordinates!
func set_dimensions(offset_dimensions: Rect2i) -> void:
	assert (_biomes != null)
	# recreate image if needed
	if _biomes.get_size() != offset_dimensions.size:
		_biomes.resize(offset_dimensions.size.x, offset_dimensions.size.y, Image.INTERPOLATE_NEAREST)
	
	_biomes.fill(Color(0.0, 0.0, 0.0, 0.0))
	_offset_dimensions = offset_dimensions
	

	
		
func set_biome(offset_coords: Vector2i, biome: StringName) -> void:
	var biome_idx : int = _biomes_mapping.get_or_add(biome, _biomes_mapping.size())
	
	var data = Color(float(biome_idx), 0.0, 0.0, 1.0)
	_biomes.set_pixelv(offset_coords - _offset_dimensions.position, data)
	
func _update_atlas() -> void:
	var textures : Array[Image] = []
	textures.resize(_biomes_mapping.size())
	for biome in _biomes_mapping:
		var image := GfxRegistry.get_biome_texture(biome)
		textures[_biomes_mapping[biome]] = image
	_biomes_atlas.create_from_images(textures)
	
func _align_self(align_with: TileMapLayer) -> void:
		# We need to shift background so that cells would match tilemap
	# _offset_dimensions.position is a displacement in offset coordinates. We need to convert
	# it into pixel coordinates
	var pixel_coorinates := align_with.map_to_local(_offset_dimensions.position)
	# However, this is coordinate of a center of a cell. We need to displace it ones more
	pixel_coorinates -= Vector2(-(CELL_SIZE * 3.0/4.0), -(sqrt(3.0)/2 * CELL_SIZE))
	#self.position = pixel_coorinates
	
func activate(align_with: TileMapLayer) -> void:

	_align_self(align_with)
	
	_update_atlas()
	var shader_material := material as ShaderMaterial
	
	shader_material.set_shader_parameter("texture_atlas", _biomes_atlas)
	shader_material.set_shader_parameter("map_data", ImageTexture.create_from_image(_biomes))
	shader_material.set_shader_parameter("cell_size", CELL_SIZE)
	shader_material.set_shader_parameter("origin", _offset_dimensions.position)
	
	# create image of this size. Its content is irrelevant as we are not going to use it, but
	# dimensions are important
	var target_dimensions := Vector2i(
		( _offset_dimensions.size.x + 0.5)  * (sqrt(3.0) * CELL_SIZE),
		(_offset_dimensions.size.y * 3.0/4.0 + 1.0/4.0) * (2 * CELL_SIZE)
		)
	print("Target offset dimensions are: %s ; pixel dimensions are: %s" % [_offset_dimensions, target_dimensions])
	var target_image := Image.create(target_dimensions.x, target_dimensions.y
	 , false, Image.FORMAT_R8)
	target_image.compress(Image.COMPRESS_ETC)
	var target_texture := ImageTexture.create_from_image(target_image)
	self.texture = target_texture
	
	

	

	
