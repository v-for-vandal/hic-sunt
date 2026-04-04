extends WorldGeneratorModuleInterface

# user-selected options
const Config = preload("res://content/worldgen-basic/worldgen/internal/heightmap_gen_config.gd")
const _VoronoizVisualization = preload("res://addons/Delaunator-GDScript/VoronoiVisualization.tscn")
const _CellClusterization := preload("res://content/worldgen-basic/worldgen/internal/cell_clusterization.gd")
const _VoronoiDrawer := preload("res://content/worldgen-basic/worldgen/internal/voronoi_drawer.gd")

const MAX_ATTEMPTS := 10000

var _config: Config
var _plane: WorldPlane
var _global_context: WorldGeneratorGlobalContext

var _terrain_noise_generator: FastNoiseLite = preload(
	"res://content/worldgen-basic/worldgen/internal/heightmap_noise_base.tres"
).duplicate()
var _mountain_noise_generator: FastNoiseLite = preload(
	"res://content/worldgen-basic/worldgen/internal/heightmap_noise_mountain.tres"
).duplicate()

var _debug_control: DebugTree.ControlInterface

var _debug_coloring_ui: OptionButton
var _debug_voronoi_viz_node: Node2D

# total size of a generated things (voronoi etc), expressed in 'points'.
# at the moment, we translate one hex to one point, but this is implementation
# detail
var _xy_dimensions := Rect2i(0, 0, 0, 0)

var _delaunay: Delaunator
var _voronoi: Voronoinator
var _tectonic_clusterization : _CellClusterization
const inter_tectonic_cluster := 1
var _continents_clusterization : _CellClusterization

# Here we store 'colors' that we use to create heightmap. Those are not really
# colors, just channels where we encode values
var _heightmap_coloring : PackedColorArray
var _heightmap : Image

var _rng := RandomNumberGenerator.new()

const RADIUS_MARGIN: int = 2

var _cube_to_map : Callable
var _map_to_cube : Callable

func _init(plane: WorldPlane, config: Variant, global_context: WorldGeneratorGlobalContext) -> void:
	super(global_context)
	_config = config
	_plane = plane
	_global_context = global_context
	_rng.seed = _global_context.seed
	
	# init conversion methods. This is a temporary measure until we decide how to
	# draw a world map
	var conversion_methods := HexagonTileMap.get_conversion_methods_for(
		TileSet.TileOffsetAxis.TILE_OFFSET_AXIS_HORIZONTAL,
		TileSet.TileLayout.TILE_LAYOUT_STACKED)
	_cube_to_map = conversion_methods.cube_to_map
	_map_to_cube = conversion_methods.map_to_cube
	
	# This is size expressed in region cells, not in regions!
	var plane_dimensions_cells_qr : Rect2i = _plane.get_cells_qr_dimensions()
	# Translate to XY
	_xy_dimensions.position = axial_to_map.call(plane_dimensions_cells_qr.position)
	_xy_dimensions.end = axial_to_map.call(plane_dimensions_cells_qr.end)
	
	_debug_control = _global_context.debug_control.add_text_node("heightmap", "")


func _ready() -> void:
	pass


func first_pass() -> void:
	_debug_control.add_text("Seed: %s" % _global_context.seed)
	_debug_control.add_text("Plane QR bbox (in regions): %s Note: one unit is region" % [_plane.get_qr_dimensions()])
	_debug_control.add_text("Plane QR bbox (in cells): %s Note: one unit is one region cell" % [_plane.get_cells_qr_dimensions()])
	_debug_control.add_text("External radius: %s" % [_plane.plane_object.get_region_external_radius()])
	_debug_control.add_text("Plane XY bbox: %s" % _xy_dimensions)
	
	# Create voronoi
	_create_voronoi()

	_create_tectonic_plates()
	_create_continents()


	_heightmap = await _render_map(_create_map())
	_plane.set_substrate(ImageTexture.create_from_image(_heightmap))
	
	var region_lambda := func(region_q: int, region_r: int, region: RegionObject) -> void:
		_region_first_pass(region, Vector2i(region_q, region_r))

	_plane.plane_object.foreach_surface(region_lambda)


func _region_first_pass(region: RegionObject, region_qrs_coords: Vector2i) -> void:
	var radius: int = _global_context.custom[&"region.radius"]
	var region_external_radius : int = _plane.plane_object.get_region_external_radius()
	var region_coords := Vector2i(
		region_qrs_coords.x * region_external_radius,
		region_qrs_coords.y * region_external_radius,
	)

	var region_cell_lambda := func(cell_q: int, cell_r: int) -> void:
		_cell_first_pass(region, region_coords, Vector2i(cell_q, cell_r))

	region.foreach(region_cell_lambda)


# region - region object
# region_global_coords - mapping of this region to heightmap and related objects
# cell_qrs_coords - coordinates of cell within this region
func _cell_first_pass(region: RegionObject, region_global_coords: Vector2i, cell_qrs_coords: Vector2i) -> void:
	var height := _get_height_at_point(region_global_coords + cell_qrs_coords)
	region.get_cell(cell_qrs_coords).get_scope().add_numeric_modifier(Modifiers.GEOGRAPHY_HEIGHT, &"wordlgen.basic", height, 0)


func _wrap_x(i: float) -> float:
	# Wrapping will be done within plane object itself
	return i


func _wrap_y(j: float) -> float:
	return j


func _create_voronoi() -> void:
	# voronoi rect is fixed and later mapped onto target rect
	var voronoi_rect := Rect2i(0, 0, 100, 100)
	var transform := Transform2D.IDENTITY.scaled( Vector2(_xy_dimensions.size) / Vector2(voronoi_rect.size))
	transform.origin = Vector2(_xy_dimensions.position)
	
	_debug_control.add_text("Voronoi to XY space transform is: %s" % transform)
	_debug_control.add_text("Verify: transform * voronoi_rect = %s" % (transform * Rect2(voronoi_rect)))
	
	var k: int = 0
	var points := PackedVector2Array()
	points.resize((voronoi_rect.size.x + 1) * (voronoi_rect.size.y+1))

	for i in range(voronoi_rect.position.x, voronoi_rect.end.x + 1):
		for j in range(voronoi_rect.position.y, voronoi_rect.end.y+1):
			var point := Vector2(i, j)
			# apply some randomness
			point.x += _rng.randfn(0.0, 3.0)
			point.y += _rng.randfn(0.0, 3.0)

			# don't allow outside of borders
			point = point.clamp(voronoi_rect.position + Vector2i(-5, -5), voronoi_rect.end + Vector2i(5, 5))

			points[k] = transform * point
			k += 1

	_delaunay = Delaunator.new(points)
	_voronoi = Voronoinator.new(_delaunay, Rect2(Vector2(_xy_dimensions.position + Vector2i(-5, -5)), Vector2(_xy_dimensions.end +  Vector2i(5, 5))))
	if _debug_control.is_debug_enabled():
		# visualization is computationally expensive, so avoid it when debug
		# is disabled://addons/debug-tree-view/internal/control_interface.gded
		_debug_voronoi_viz_node = _VoronoizVisualization.instantiate()
		_debug_voronoi_viz_node.set_voronoi(_voronoi)
		_debug_voronoi_viz_node.add_random_coloring(&"base", Vector3(-1.0, -1.0, 1.0))

		_debug_coloring_ui = OptionButton.new()
		_debug_coloring_ui.add_item("base")
		var coloring_selector_fn := func(index: int) -> void:
			if index != -1:
				var text := _debug_coloring_ui.get_item_text(index)
				_debug_voronoi_viz_node.select_coloring(text)

		_debug_coloring_ui.item_selected.connect(coloring_selector_fn)

		_debug_control.add_2d_node("voronoi", _debug_voronoi_viz_node, Rect2i(0, 0, 100, 100), _debug_coloring_ui)


func _create_tectonic_plates() -> void:
	assert(_voronoi.voronoi_cells.size() > 0)

	var clusterization := _CellClusterization.new(_voronoi)
	clusterization.rng = _rng


	const tectonic_section_start := 2

	var tectonic_clusters := clusterization.seed_n_clusters(4, tectonic_section_start, true)

	# now, start expanding
	var has_expanded := true
	while has_expanded:
		has_expanded = false
		for cluster_id: int in tectonic_clusters:
			# expand this cluster
			var neighbours := clusterization.cluster_neighbours(cluster_id)

			for neighbour: int in neighbours:
				if clusterization.cell_cluster(neighbour) != 0:
					# already taken, continue
					continue
				if clusterization.is_neighbour_to_any_cluster(neighbour, [cluster_id, inter_tectonic_cluster]):
					# it has other neighbours. In this case, mark is as inter-tectonic
					clusterization.add_to_cluster(neighbour, inter_tectonic_cluster)
					has_expanded = true # not strictly true, but helps with algorithm
					continue

				clusterization.add_to_cluster(neighbour, cluster_id)
				has_expanded = true
	_tectonic_clusterization = clusterization
	
	if _debug_voronoi_viz_node != null:
		# add coloring for tectonic plates
		var cluster_colors: Dictionary[int, Color] = {
			inter_tectonic_cluster: Color("#fe663e"),
			0: Color("#ff3df3"),
		}

		for cluster_id: int in range(tectonic_section_start, tectonic_section_start + clusterization.clusters_count()):
			cluster_colors[cluster_id] = Color(0.3, randf(), randf())

		var tectonic_coloring := PackedColorArray()
		tectonic_coloring.resize(_voronoi.voronoi_cells.size())
		for i in range(_voronoi.voronoi_cells.size()):
			tectonic_coloring[i] = cluster_colors[clusterization.cell_cluster(i)]

		_debug_add_coloring("tectonic", tectonic_coloring)

func _create_continents() -> void:
	var clusterization := _CellClusterization.new(_voronoi)
	clusterization.rng = _rng
	
	var continent_clusters := clusterization.seed_n_clusters(10, 1, false)
	
	var surface_cells_count := 0
	var next_cluster_id := 1
	
	const  cells_per_iteration := 10
	
	while surface_cells_count < 0.3 * _voronoi.voronoi_cells.size():
		# grow next continent a bit
		var cluster_id := next_cluster_id
		# formula is 1 + ( (next_cluster_id + 1 - 1) % clusters_count)
		next_cluster_id = 1 + ( next_cluster_id % continent_clusters.size())
		var neighbours := clusterization.cluster_neighbours(cluster_id)
		
		var selected_neighbours : Array[int] = []

		for neighbour: int in neighbours:
				if clusterization.cell_cluster(neighbour) != 0:
					# already taken, continue
					continue
				selected_neighbours.append(neighbour)

		if selected_neighbours.size() > cells_per_iteration:
			selected_neighbours.shuffle()
			selected_neighbours.slice(0, cells_per_iteration)
		
		for neighbour: int in selected_neighbours:
			clusterization.add_to_cluster(neighbour, cluster_id)
			surface_cells_count += 1
	
	_continents_clusterization = clusterization
	
	if _debug_voronoi_viz_node != null:
		# add coloring for tectonic plates
		var cluster_colors: Dictionary[int, Color] = {
			0: Color.BLUE,
		}

		for cluster_id: int in range(1, 1 + clusterization.clusters_count()):
			cluster_colors[cluster_id] = Color(randf(), randf(), 0.1)

		var continent_coloring := PackedColorArray()
		continent_coloring.resize(_voronoi.voronoi_cells.size())
		for i in range(_voronoi.voronoi_cells.size()):
			continent_coloring[i] = cluster_colors[clusterization.cell_cluster(i)]

		_debug_add_coloring("continents", continent_coloring)
		
func _encode_as_color(cell_index: int) -> Color:
	var cell_cluster := _continents_clusterization.cell_cluster(cell_index)
	# TODO: This is debug
	if cell_cluster == 0:
		# when we render, Godot will perform srgb_to_linear conversion
		# to mak inverse conversion first
		return Color(-100, 0, 0, 1.0).linear_to_srgb()
	else:
		return Color(100.0, 0.0, 0.0, 1.0).linear_to_srgb()
	
func _create_map() -> _VoronoiDrawer:
	var colors := PackedColorArray()
	var size := _voronoi.voronoi_cells.size()
	colors.resize(size)
	for i in range(size):
		colors[i] = _encode_as_color(i)
	
	var result := _VoronoiDrawer.new(_voronoi, colors)
	return result
	
func _render_map(map: _VoronoiDrawer) -> Image:
	var viewport := SubViewport.new()	
	viewport.disable_3d = true
	viewport.use_hdr_2d = true
	viewport.transparent_bg = true
	viewport.gui_disable_input = true
	viewport.size = _xy_dimensions.size
	viewport.render_target_update_mode = SubViewport.UPDATE_ONCE
	
	var transform := Transform2D()
	transform.origin = -1 * Vector2(_xy_dimensions.position)
	viewport.set_canvas_transform(transform)

	viewport.add_child(map)
	add_child(viewport)
	

	await map.finished_drawing
	await RenderingServer.frame_post_draw
	
	var result := viewport.get_texture().get_image()
	
	_debug_control.add_image_node("map", result)
	
	return result
	

	

func _debug_add_coloring(key: StringName, coloring: PackedColorArray) -> void:
	_debug_voronoi_viz_node.add_coloring(key, coloring)
	_debug_coloring_ui.add_item(key)

# TODO: Make some other formula
var _MOUNTAIN_EXTRA_HEIGHT_RANGE := Vector2i(1000, 2000)


func _displacement_at_range(value: float, target_range: Vector2i) -> float:
	return (value - target_range.x) / (target_range.y - target_range.x)


func _get_height_at_point(cell_global_qr_coords: Vector2i) -> int:
	var image_coords := axial_to_map(cell_global_qr_coords)
	var color := _heightmap.get_pixelv(image_coords)
	var height := int(color.r)
	
	return height


func axial_to_map(qr_coords: Vector2i) -> Vector2i:
	var qrs_coords := Vector3i(qr_coords.x, qr_coords.y, - qr_coords.x - qr_coords.y)
	return _cube_to_map.call(qrs_coords)
	
func map_to_axial(xy_coords: Vector2i) -> Vector2i:
	var qrs_coords := _map_to_cube.call(xy_coords) as Vector3i
	return Vector2i(qrs_coords.x, qrs_coords.y)
