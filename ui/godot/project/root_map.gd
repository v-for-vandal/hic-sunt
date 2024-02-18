extends Node2D

var _loaded := false
var _region_map_scene : PackedScene = load("res://game/region/region_map.tscn") 
var _world_map_scene : PackedScene = load("res://game/world/world_map.tscn") 
var _world_map: Node
var _region_map: Node


# Should be called manually to propertly initialize object
func root_load():
	_world_map = _world_map_scene.instantiate()
	_region_map = _region_map_scene.instantiate()
	_world_map.show_region_request.connect(_on_world_map_show_region_request)
	_region_map.exit_reqion_request.connect(_on_region_map_exit_reqion_request)
	
	GameUiEventBus.set_world_interaction(_world_map)
	GameUiEventBus.set_region_interaction(_region_map)
	_switch_to_world()
	_loaded = true
	
func _build_tileset(sources: Array):
	pass
	
#func _build_tiles_mapping(world_object: WorldObject):
	#assert(world_object != null)
	#var terrain_types:Array = CurrentGame.current_player_ruleset.get_terrain_types()
	#var result := {}
	#
	#for terrain in terrain_types:
		#result[terrain.id] = terrain.render.atlas_coords
	#
	#return result
		
	


func load_world(world_object : WorldObject):
	assert(_loaded, "You can't call methods on root-map before it is fully loaded")
	assert(world_object != null)
	# TODO: Don't set up terrain mapping, instead use it as global class
	var terrain_mapping = CurrentGame.get_atlas_visualization()
	_world_map.set_terrain_visualization(terrain_mapping)
	_region_map.set_terrain_visualization(terrain_mapping)
	_world_map.load_world(world_object)
	_switch_to_world()


func _on_world_map_show_region_request(world_object:  WorldObject, qr_position: Vector2i):
	assert(_loaded, "THis instance can't react to signals before it is fully loaded")
	assert(_region_map != null, "_region_map is somehow null")
	var region_obj = world_object.get_region(qr_position);
	# TODO: Check for fog
	if region_obj != null:
		_region_map.load_region(region_obj)
	else:
		print("region is absent")
	_switch_to_region()
	
func _switch(from: Node, to: Node):
	if from.get_parent() != null:
		assert(from.get_parent() == self)
		remove_child(from)
		
	if to.get_parent() == null:
		add_child(to)
	else:
		assert(to.get_parent() == self)
	
func _switch_to_region():
	_switch(_world_map, _region_map)
		
func _switch_to_world():
	_switch(_region_map, _world_map)


func _on_region_map_exit_reqion_request():
	assert(_loaded, "THis instance can't react to signals before it is fully loaded")
	_switch_to_world()
	
func _unhandled_input(event: InputEvent) -> void:
	if event.is_action("ui_cancel"):
		if event.is_action_released("ui_cancel"):
			#print("sending cancellation event")
			GameUiEventBus.emit_cancellation()
		get_viewport().set_input_as_handled()
		
		
