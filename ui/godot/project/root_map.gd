extends Node2D

#var _loaded := false


@onready var _world_map: Node = $WorldMap
@onready var _region_map: Node = $RegionMap

func _ready() -> void:
	assert(_world_map != null, "Child failed to initialize")
	assert(_region_map != null, "Child failed to initialize")
	GameUiEventBus.set_world_interaction(_world_map)
	GameUiEventBus.set_region_interaction(_region_map)
	# world is empty at the moment, but lets set up everything properly
	_switch_to_world()
	
	# TODO: RM - was checking how get_script works
	#print("Check get_script.get_resource_path")
	#print(_region_map.get_script().resource_path)
	#print(_world_map.get_script().resource_path)
	#var test_script = ResourceLoader.load(_region_map.get_script().resource_path)
	#var test_object = test_script.new()
	#print(test_object)

	

#func _build_tiles_mapping(world_object: WorldObject):
	#assert(world_object != null)
	#var terrain_types:Array = CurrentGame.current_player_ruleset.get_terrain_types()
	#var result := {}
	#
	#for terrain in terrain_types:
		#result[terrain.id] = terrain.render.atlas_coords
	#
	#return result
		
	


func load_world(world_object : WorldObject) -> void:
	#assert(_loaded, "You can't call methods on root-map before it is fully loaded")
	assert(world_object != null)
	# TODO: Don't set up terrain mapping, instead use it as global class
	var terrain_mapping : Dictionary = CurrentGame.get_atlas_visualization()
	_world_map.set_terrain_visualization(terrain_mapping)
	_region_map.set_visualization(terrain_mapping)
	_world_map.load_world(world_object)
	_switch_to_world()


func _on_world_map_show_region_request(world_object:  WorldObject, qr_position: Vector2i) -> void:
	#assert(_loaded, "THis instance can't react to signals before it is fully loaded")
	assert(_region_map != null, "_region_map is somehow null")
	var region_obj = world_object.get_region(qr_position);
	
	if region_obj == null:
		print("region is absent")
		return
		
	# We need to add scene to the tree before we can call load_region
	_switch_to_region()
	_region_map.load_region(region_obj)
	
	
func _switch(from: Node, to: Node) -> void:
	if from.get_parent() != null:
		assert(from.get_parent() == self)
		remove_child(from)
		
	if to.get_parent() == null:
		add_child(to)
	else:
		assert(to.get_parent() == self)
	
func _switch_to_region() -> void:
	_switch(_world_map, _region_map)
		
func _switch_to_world() -> void:
	_switch(_region_map, _world_map)


func _on_region_map_exit_reqion_request() -> void:
	#assert(_loaded, "THis instance can't react to signals before it is fully loaded")
	_switch_to_world()

	
func _unhandled_input(event: InputEvent) -> void:
	if event.is_action("ui_cancel"):
		if event.is_action_released("ui_cancel"):
			#print("sending cancellation event")
			GameUiEventBus.emit_cancellation()
		get_viewport().set_input_as_handled()
		
		
