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

#var _terrain_noise_generator: FastNoiseLite = preload(
	#"res://content/worldgen-basic/worldgen/internal/heightmap_noise_base.tres"
#).duplicate()
#var _mountain_noise_generator: FastNoiseLite = preload(
	#"res://content/worldgen-basic/worldgen/internal/heightmap_noise_mountain.tres"
#).duplicate()

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
const INTER_TECTONIC_CLUSTER := 1
var _continents_clusterization : _CellClusterization
var _distance_to_shore_clusterization : _CellClusterization

var _heightmap : Image
var _heightmap_transformation : Transform2D
var _heightmap_debug_node : DebugTree.ControlInterface

var _debug_used_rect : Rect2i = Rect2i(Vector2i.ZERO, Vector2i.ONE)

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
	_calculate_distance_to_shore()
	
	_heightmap = await _render_map(_create_map())
	
	_create_heightmap_transformation()


	_plane.set_substrate(ImageTexture.create_from_image(_heightmap))
	
	_heightmap_debug_node = _debug_control.add_image_node("region_map", _heightmap)

	
	var region_lambda := func(region_q: int, region_r: int, region: RegionObject) -> void:
		_region_first_pass(region, Vector2i(region_q, region_r))

	_plane.plane_object.foreach_surface(region_lambda)
	
	_debug_add_used_heighmap_part()


func _region_first_pass(region: RegionObject, region_qrs_coords: Vector2i) -> void:
	var region_external_radius : int = _plane.plane_object.get_region_external_radius()
	# position of the region center in global cell coordinate system
	var region_center_cell_coords := _plane.convert_to_cells_qr(region_qrs_coords)

	
	# first, lets calculate approximate region height. We are not going to bother with
	# exact math, like trying to avoid cells from other regions. Just quick and dirty
	
	var cell_heights : Array[int] = []
	
	# for debugging :(
	var region_debug_rect := Rect2i( _heightmap_transformation * Vector2(region_center_cell_coords), Vector2i.ONE)
	for x in range(- region_external_radius, region_external_radius, 3):
		for y in range(- region_external_radius,  region_external_radius, 3):
			var height := _get_height_at_point(region_center_cell_coords + Vector2i(x,y))
			cell_heights.append(height)
			
			# debug
			var coords := Vector2i(_heightmap_transformation * Vector2(region_center_cell_coords + Vector2i(x,y)))
			region_debug_rect = region_debug_rect.expand(coords)
		
	# sort all heights
	var region_height := 0
	cell_heights.sort()
	# get 75 percentile
	var idx75 := clampi(int(cell_heights.size() * 0.75), 0, cell_heights.size())
	if cell_heights.size() > 0:
		region_height = cell_heights[idx75]
		region.get_scope().add_numeric_modifier(Modifiers.GEOGRAPHY_HEIGHT,  &"wordlgen.basic", region_height, 0)

	var region_cell_lambda := func(cell_q: int, cell_r: int) -> void:
		_cell_first_pass(region, region_center_cell_coords, Vector2i(cell_q, cell_r), region_height )

	region.foreach(region_cell_lambda)
	
	# add as overlay
	if region_qrs_coords.x % 10 == 0 and region_qrs_coords.y % 10 == 0:
		var display_rect := Rect2i(
			region_center_cell_coords - Vector2i(region_external_radius, region_external_radius),
			region_center_cell_coords + Vector2i(region_external_radius, region_external_radius)
		)
		# Something is very wrong with this transformation :(
		display_rect = Rect2i(_heightmap_transformation * Rect2(display_rect))
		
		_heightmap_debug_node.add_to_overlay(DebugTree.create_rect_overlay(
			#display_rect,
			region_debug_rect,
			Color.ORANGE,
			region.get_id()
		))



# region - region object
# region_global_coords - mapping of this region to heightmap and related objects
# cell_qrs_coords - coordinates of cell within this region
# region_height - height of the region. 
func _cell_first_pass(region: RegionObject, region_global_coords: Vector2i, cell_qrs_coords: Vector2i, region_height: int) -> void:
	var height := _get_height_at_point(region_global_coords + cell_qrs_coords)
	# we want our height relative to region height
	height -= region_height
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
				if clusterization.is_neighbour_to_any_cluster(neighbour, [cluster_id, INTER_TECTONIC_CLUSTER]):
					# it has other neighbours. In this case, mark is as inter-tectonic
					clusterization.add_to_cluster(neighbour, INTER_TECTONIC_CLUSTER)
					has_expanded = true # not strictly true, but helps with algorithm
					continue

				clusterization.add_to_cluster(neighbour, cluster_id)
				has_expanded = true
	_tectonic_clusterization = clusterization
	
	if _debug_voronoi_viz_node != null:
		# add coloring for tectonic plates
		var cluster_colors: Dictionary[int, Color] = {
			INTER_TECTONIC_CLUSTER: Color("#fe663e"),
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
		
func _calculate_distance_to_shore() -> void:
	# Build distance-to-sea-shore. Idea is this: create a new clusterization with
	# one cluster. In that cluster we will put all neighbours of a all continents.
	# Then we will do BFS to iterate one layer by one layer. This cluster will grow both into the continents
	# and into the sea, one layer at a time. At each layer we will put iteration number into cell metadata
	# and that iteration number is essentially distance from the sealine.
	const START_DISTANCE_CLUSTER := 1
	assert(_continents_clusterization != null)
	var clusterization := _CellClusterization.new(_voronoi)
	for continent_cluster_id in _continents_clusterization.get_cluster_ids():
		var continent_neighbours := _continents_clusterization.cluster_neighbours(continent_cluster_id)
		for neighbour in continent_neighbours:
			clusterization.add_to_cluster(neighbour, START_DISTANCE_CLUSTER)
			
	# Because we have only covered sea cells at the moment, we need to add shore cells to this cluster
	# as well
	var neighbours := clusterization.cluster_neighbours(START_DISTANCE_CLUSTER)
	for neighbour in neighbours:
		# if continent
		if _continents_clusterization.cell_cluster(neighbour) != 0:
			clusterization.add_to_cluster(neighbour, START_DISTANCE_CLUSTER)
			
	# Now, we have proper start point.
	var next_distance := START_DISTANCE_CLUSTER + 1
	var next_cluster_cells := clusterization.cluster_neighbours(START_DISTANCE_CLUSTER)
	while(next_cluster_cells.size() > 0):
		for cell_id in next_cluster_cells:
			if not clusterization.is_cell_in_any_cluster(cell_id):
				clusterization.add_to_cluster(cell_id, next_distance)
		
		if clusterization.has_cluster_with_id(next_distance):
			next_cluster_cells = clusterization.cluster_neighbours(next_distance)
		else:
			next_cluster_cells = []
		next_distance += 1
		
	_distance_to_shore_clusterization = clusterization
		
	if _debug_voronoi_viz_node != null:
		# add coloring for distance-to-shore plates
		var cluster_colors: Dictionary[int, Color] = {
			0: Color.BLUE,
		}

		for cluster_id: int in range(1, 1 + clusterization.clusters_count()):
			cluster_colors[cluster_id] = Color(randf(), randf(), 0.1)

		var distance_coloring := PackedColorArray()
		distance_coloring.resize(_voronoi.voronoi_cells.size())
		for i in range(_voronoi.voronoi_cells.size()):
			distance_coloring[i] = cluster_colors[clusterization.cell_cluster(i)]

		_debug_add_coloring("distance", distance_coloring)
	
func _encode_as_color(cell_index: int) -> Color:
	var cell_continent := _continents_clusterization.cell_cluster(cell_index)
	var cell_tectonic := _tectonic_clusterization.cell_cluster(cell_index)
	var distance_to_shore := _distance_to_shore_clusterization.cell_cluster(cell_index)
	
	var height_min := 0
	var height_max := 1
	var height_95_min := 0.5
	var height_95_max := 0.5
	# base range, without mountains, is different for land and for sea. For land it goes
	# between 10 and 1000 and the min-max goes hight the further we are from sea
	if cell_continent != 0:
		# land
		height_min = 10
		height_max = 1000
		height_95_max = clampf(distance_to_shore * 100.0, height_min, height_max * 0.75)
		height_95_min = clampf(distance_to_shore * 75, height_min, height_max * 0.6)
	else:
		# ocean
		height_min = -4000
		height_max =  -10
		height_95_min = clampf(distance_to_shore * -500, height_min, height_max)
		height_95_max = clampf(distance_to_shore * -300, height_min, height_max)
		
		
	
	# base height
	var height_mean := (height_95_min + height_95_max) / 2.0
	# 2 sigmas in every direction is 95%
	var height_std := (height_95_max - height_95_min) / 4.0
	
	var height := clampf( _rng.randfn(height_mean, height_std), height_min, height_max)


	# add mountains in tectonic places
	if cell_tectonic == INTER_TECTONIC_CLUSTER:
		height += _rng.randfn(2000, 1000)

	# when we render, Godot will perform srgb_to_linear conversion
	# So we make inverse conversion first
	return Color(height, 0.0, 0.0, 1.0).linear_to_srgb()
	
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
	

	
	
func _create_heightmap_transformation() -> void:
	assert (_heightmap != null, "heightmap is not initialized")
	var target_rect := Rect2(Vector2.ZERO, Vector2(_heightmap.get_size()))
	var source_rect := Rect2(_plane.get_cells_qr_dimensions())
	
	var scale := target_rect.size / source_rect.size
	
	_heightmap_transformation = Transform2D()
	_heightmap_transformation = _heightmap_transformation.translated(-source_rect.position)
	_heightmap_transformation = _heightmap_transformation.scaled(scale)
	_heightmap_transformation = _heightmap_transformation.translated(target_rect.position)
	

func _debug_add_coloring(key: StringName, coloring: PackedColorArray) -> void:
	_debug_voronoi_viz_node.add_coloring(key, coloring)
	_debug_coloring_ui.add_item(key)
	
func _debug_add_used_heighmap_part() -> void:
	assert(_heightmap_debug_node != null)
	
	# Something is very wrong with this conversion
	#var source_rect := _plane.get_cells_qr_dimensions()
	#var target_rect := _heightmap_transformation * Rect2(source_rect)
	
	_heightmap_debug_node.add_to_overlay(DebugTree.create_rect_overlay(Rect2i(_debug_used_rect), Color.ORANGE, "used_part"))

func _displacement_at_range(value: float, target_range: Vector2i) -> float:
	return (value - target_range.x) / (target_range.y - target_range.x)


func _get_height_at_point(cell_global_qr_coords: Vector2i) -> int:
	# var image_coords := axial_to_map(cell_global_qr_coords)
	var image_coords := _heightmap_transformation * Vector2(cell_global_qr_coords)
	_debug_used_rect = _debug_used_rect.expand(image_coords)
	var color := _heightmap.get_pixelv(image_coords)
	var height := int(color.r)
	
	return height


func axial_to_map(qr_coords: Vector2i) -> Vector2i:
	var qrs_coords := Vector3i(qr_coords.x, qr_coords.y, - qr_coords.x - qr_coords.y)
	return _cube_to_map.call(qrs_coords)
	
func map_to_axial(xy_coords: Vector2i) -> Vector2i:
	var qrs_coords := _map_to_cube.call(xy_coords) as Vector3i
	return Vector2i(qrs_coords.x, qrs_coords.y)
