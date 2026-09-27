extends Node

const ROOT_MAP_SCENE := preload("res://root_map.tscn")

const PLANE_ID := &"sample_main"
# PlaneObject converts Rect2i.position and Rect2i.end directly into native QRSBox.
# Native QRSBox has inclusive end coordinates, so size (1, 1) means regions 0..1 × 0..1.
const WORLD_SIZE := Rect2i(Vector2i(0, 0), Vector2i(1, 1))
const REGION_RADIUS := 3
const TUNDRA_BIOME := &"core.biome.tundra"
const SAMPLE_MODIFIER := &"sample.ui_world_map"

var _root_map: Node
var _world: World
var _ruleset: RulesetObject


func _ready() -> void:
	_ruleset = CentralSystem.load_ruleset()
	_world = _create_sample_world()

	CurrentGame.init_game(_world, _ruleset)
	_create_sample_civilisations_and_cities()
	_load_world_map_view()

	print("UI world map sample is ready: 4 tundra regions, 3 civilizations, 3 cities")


func _create_sample_world() -> World:
	var world := World.new()
	var plane := world.create_plane(PLANE_ID, WORLD_SIZE, REGION_RADIUS)
	assert(plane != null, "Failed to create sample plane")
	if plane == null:
		return world

	print("3_civs sample world dimensions: ", plane.get_qr_dimensions())
	_fill_world_with_tundra(plane)
	plane.set_substrate(_create_tundra_substrate())
	return world


func _fill_world_with_tundra(plane: WorldPlane) -> void:
	var region_lambda := func(region_q: int, region_r: int, region: RegionObject) -> void:
		_fill_region_with_tundra(region, Vector2i(region_q, region_r))

	plane.foreach_surface(region_lambda)


func _fill_region_with_tundra(region: RegionObject, region_coords: Vector2i) -> void:
	var region_scope := region.get_scope()
	region_scope.add_numeric_modifier(Modifiers.GEOGRAPHY_HEIGHT, SAMPLE_MODIFIER, 100.0, 0.0)
	region_scope.add_numeric_modifier(Modifiers.ECOSYSTEM_TEMPERATURE, SAMPLE_MODIFIER, -5.0, 0.0)
	region_scope.add_numeric_modifier(Modifiers.ECOSYSTEM_PRECIPITATION, SAMPLE_MODIFIER, 120.0, 0.0)

	var cell_lambda := func(cell_q: int, cell_r: int) -> void:
		var cell := region.get_cell(Vector2i(cell_q, cell_r))
		if cell == null or not cell.is_valid():
			return

		var scope := cell.get_scope()
		scope.add_numeric_modifier(Modifiers.GEOGRAPHY_HEIGHT, SAMPLE_MODIFIER, 100.0, 0.0)
		scope.add_numeric_modifier(Modifiers.ECOSYSTEM_TEMPERATURE, SAMPLE_MODIFIER, -5.0, 0.0)
		scope.add_numeric_modifier(Modifiers.ECOSYSTEM_PRECIPITATION, SAMPLE_MODIFIER, 120.0, 0.0)
		scope.add_string_modifier(
			Modifiers.ECOSYSTEM_BIOME,
			SAMPLE_MODIFIER,
			TUNDRA_BIOME,
			Modifiers.ECOSYSTEM_BIOME_MAX_LEVEL
		)

	region.foreach(cell_lambda)


func _create_tundra_substrate() -> ImageTexture:
	var image := Image.create(8, 8, false, Image.FORMAT_RGBA8)
	image.fill(Color(0.72, 0.78, 0.82, 1.0))
	return ImageTexture.create_from_image(image)


func _create_sample_civilisations_and_cities() -> void:
	var plane := _world.get_plane(PLANE_ID)
	assert(plane != null, "Sample plane was not found")
	if plane == null:
		return

	var city_specs: Array[Dictionary] = [
		{"civ": CurrentGame.game.get_current_player_civ(), "coords": Vector2i(0, 0)},
		{"civ": Civilisation.create_civilisation(&"sample.civ_1"), "coords": Vector2i(1, 0)},
		{"civ": Civilisation.create_civilisation(&"sample.civ_2"), "coords": Vector2i(0, 1)},
	]

	for spec: Dictionary in city_specs:
		var civ: Civilisation = spec["civ"]
		var coords: Vector2i = spec["coords"]
		if civ == null:
			push_error("Failed to create sample civilisation for region %s" % [coords])
			continue

		var region := plane.plane_object.get_region(coords)
		assert(region != null, "Sample region is missing")
		if region == null:
			continue

		var city_cell := region.get_cell(Vector2i.ZERO)
		var create_result := civ.create_city(city_cell)
		if not create_result.is_ok():
			push_error("Failed to create sample city in region %s: %s" % [coords, create_result.errors])
			continue

		region.set_city_id(create_result.city.get_id())


func _load_world_map_view() -> void:
	_root_map = ROOT_MAP_SCENE.instantiate()
	add_child(_root_map)
	_root_map.load_world(_world)
