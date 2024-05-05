extends Control

# private members
var _project_holder_scene = preload("res://ui/city/city_project_holder.tscn")
var _queue : CityProjectsQueue

func _ready() -> void:
	var children = get_children(true)
	var root_container = $%RootContainer
	print("Children: ", children)
	print("Root container: ", $%RootContainer)
	print("get_node Root container", self.get_node("RootContainer"))
	print("get_node as unique Root container", self.get_node("%RootContainer"))
	assert($%RootContainer != null, "Something wrong, child is missing")
	
func _init() -> void:
	print("Initializing CityProjectsQueue")
	
	
func _enter_tree() -> void:
	print("CityProjectsQueue is entering tree")


func setup(queue: CityProjectsQueue) -> void:
	_queue = queue
	for idx : int in range(0, queue.size()):
		_add_project_widget(idx)
		
	# connect signals
	queue.removed.connect(_on_removed)
	queue.swapped.connect(_on_swapped)
	queue.inserted.connect(_add_project_widget)
	
func _add_project_widget(idx: int) -> void:
	var project := _queue.get_project_at(idx)
	var project_holder = _project_holder_scene.instantiate()
	project_holder.setup(project)
	$%RootContainer.add_child(project_holder)
	$%RootContainer.move_child(project_holder, idx)
	
func _on_removed(idx: int) -> void:
	var to_rm := $%RootContainer.get_child(idx)
	if to_rm == null:
		push_error("attempt to remove non-existent child at index ", idx)
		return
	$%RootContainer.remove_child(to_rm)
	to_rm.queue_free()
	
func _on_swapped(idx1: int, idx2: int) -> void:
	assert(idx1 != idx2, "Indexes are equal, no reason to swap")
	var min_idx : int = min(idx1, idx2)
	var max_idx : int = max(idx1, idx2)
	
	# 0 1 2 3 4 5
	# a b c d e f
	# swap b(1) <-> e(4)
	# move b -> 4
	#
	# 0 1 2 3 4 5
	# a c d e b f
	# 
	# e is now (4-1)=3
	# move e->1
	#
	# 0 1 2 3 4 5
	# a e c d b f
	$%RootContainer.move_child(get_child(min_idx), max_idx)
	$%RootContainer.move_child(get_child(max_idx-1), min_idx)
	
func _clear() -> void:
	var children = get_children()
	for child in children:
		remove_child(child)
		child.queue_free()
		
func load_region(region: RegionObject) -> void:
	_clear()
	var city_id_opt := region.get_city_id()
	if city_id_opt.is_empty():
		return
		
	# TODO: This will not permit displaying cities of other civs
	var city : City = CurrentGame.get_current_player_civ().find_city_by_id(city_id_opt)
	
	if city == null:
		push_error("Can't fetch city with id: ", city_id_opt)
		return
		
	setup(city.get_projects_queue())
