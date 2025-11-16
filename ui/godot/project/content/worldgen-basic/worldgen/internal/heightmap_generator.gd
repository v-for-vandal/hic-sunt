extends WorldGeneratorModuleInterface

# user-selected options
const Config = preload("res://content/worldgen-basic/worldgen/internal/heightmap_gen_config.gd")
const _VoronoizVisualization = preload("res://addons/Delaunator-GDScript/VoronoiVisualization.tscn")
const _CellClusterization := preload("res://content/worldgen-basic/worldgen/internal/cell_clusterization.gd")
const _VoronoiDrawer := preload("res://content/worldgen-basic/worldgen/internal/voronoi_drawer.gd")

const MAX_ATTEMPTS := 10000

var _config: Config
var _plane: PlaneObject
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

var _delaunay: Delaunator
var _voronoi: Voronoinator
var _tectonic_clusterization : _CellClusterization
const inter_tectonic_cluster := 1
var _continents_clusterization : _CellClusterization

# Here we store 'colors' that we use to create heightmap. Those are not really
# colors, just channels where we encode values
var _heightmap_coloring : PackedColorArray

var _rng := RandomNumberGenerator.new()

const RADIUS_MARGIN: int = 2


func _init(plane: PlaneObject, config: Variant, global_context: WorldGeneratorGlobalContext) -> void:
	super(global_context)
	_config = config
	_plane = plane
	_global_context = global_context
	_rng.seed = _global_context.seed

	_debug_control = _global_context.debug_control.add_text_node("heightmap", "")


func _ready() -> void:
	pass


func first_pass() -> void:
	# Create voronoi
	_create_voronoi()

	_create_tectonic_plates()
	_create_continents()

	var map := _create_map()
	await _render_map(map)
	
	var region_lambda := func(region_q: int, region_r: int, region: RegionObject) -> void:
		_region_first_pass(region, Vector2i(region_q, region_r))

	_plane.foreach_surface(region_lambda)


func _region_first_pass(region: RegionObject, region_qrs_coords: Vector2i) -> void:
	var radius: int = _global_context.custom[&"region.radius"]
	var region_coords := Vector2i(
		region_qrs_coords.x * (radius + RADIUS_MARGIN),
		region_qrs_coords.y * (radius + RADIUS_MARGIN),
	)

	var region_cell_lambda := func(cell_q: int, cell_r: int) -> void:
		_cell_first_pass(region, region_coords, Vector2i(cell_q, cell_r))

	region.foreach(region_cell_lambda)


func _cell_first_pass(region: RegionObject, region_coords: Vector2i, cell_qrs_coords: Vector2i) -> void:
	var height := _get_height_at_point(region_coords.x + cell_qrs_coords.x, region_coords.y + cell_qrs_coords.y)
	region.get_cell(cell_qrs_coords).get_scope().add_numeric_modifier(Modifiers.GEOGRAPHY_HEIGHT, &"wordlgen.basic", height, 0)


func _wrap_x(i: float) -> float:
	# Wrapping will be done within plane object itself
	return i


func _wrap_y(j: float) -> float:
	return j


func _create_voronoi() -> void:
	var max_i := 100
	var max_j := 100
	var k: int = 0
	var points := PackedVector2Array()
	points.resize(max_i * max_j)

	for i in range(0, 100):
		for j in range(0, 100):
			var point := Vector2(i, j)
			# apply some randomness
			point.x += _rng.randfn(0.0, 3.0)
			point.y += _rng.randfn(0.0, 3.0)

			# don't allow outside of borders
			point = point.clamp(Vector2(-5, -5), Vector2(max_i + 5, max_j + 5))

			points[k] = point
			k += 1

	_delaunay = Delaunator.new(points)
	_voronoi = Voronoinator.new(_delaunay, Rect2(Vector2(-5, -5), Vector2(110, 110)))
	if _debug_control.is_debug_enabled():
		# visualization is computationally expensive, so avoid it when debug
		# is disablres://addons/debug-tree-view/internal/control_interface.gded
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
		return Color.BLACK
	else:
		return Color.YELLOW
	
func _create_map() -> _VoronoiDrawer:
	var colors := PackedColorArray()
	var size := _voronoi.voronoi_cells.size()
	colors.resize(size)
	for i in range(size):
		colors[i] = _encode_as_color(i)
	
	var result := _VoronoiDrawer.new(_voronoi, colors)
	# TODO: RM
	#var copy := _VoronoiDrawer.new(_voronoi, colors)
	#_debug_control.add_2d_node("drawer", copy)
	return result
	
func _render_map(map: _VoronoiDrawer) -> void:
	var viewport := SubViewport.new()	
	viewport.disable_3d = true
	viewport.size = Vector2i(100, 100) # TODO: Make a setting
	viewport.render_target_update_mode = SubViewport.UPDATE_ONCE

	viewport.add_child(map)
	add_child(viewport)
	
	await RenderingServer.frame_post_draw
	await map.finished_drawing
	
	var result := viewport.get_texture().get_image()
	
	_debug_control.add_image_node("map", result)
	

	

func _debug_add_coloring(key: StringName, coloring: PackedColorArray) -> void:
	_debug_voronoi_viz_node.add_coloring(key, coloring)
	_debug_coloring_ui.add_item(key)

# TODO: Make some other formula
var _MOUNTAIN_EXTRA_HEIGHT_RANGE := Vector2i(1000, 2000)


func _displacement_at_range(value: float, target_range: Vector2i) -> float:
	return (value - target_range.x) / (target_range.y - target_range.x)


func _get_height_at_point(i: float, j: float) -> int:
	i = _wrap_x(i)
	j = _wrap_y(j)
	# Get base height. It will be in range (-1, 1)
	var sample_value := _terrain_noise_generator.get_noise_2d(i, j)

	var height_meters := NoiseToolsLibrary.sample_simple_range_i(sample_value, _config.height_range)

	# add mountains
	var mountain_sample_value := _mountain_noise_generator.get_noise_2d(i, j)
	var mountain_extra_meters := 0 # by default, don't add anything
	if mountain_sample_value > 0.75:
		# renormalize to range (-1,1) because that is what _sample_simple_range
		# expects
		mountain_sample_value = -1 + (mountain_sample_value - 0.75) / (1 - 0.75) * 2
		mountain_sample_value = NoiseToolsLibrary.sample_simple_range_i(mountain_sample_value, _MOUNTAIN_EXTRA_HEIGHT_RANGE)

	var result := int(clamp(height_meters + mountain_extra_meters, _config.height_range.x, _config.height_range.y))

	return result
