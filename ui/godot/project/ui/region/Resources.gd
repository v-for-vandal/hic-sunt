extends GridContainer

var _region_object: RegionObject

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.

func _clear() -> void:
	if _region_object == null:
		return

	_disconnect_region_object(_region_object)
	_clear_widgets()
	
func _clear_widgets() -> void:
	for ch in get_children():
		remove_child(ch)
		ch.queue_free()
		
func _create_resource_name_node(res_id:String) -> Label:
	var result := Label.new()
	result.text = res_id
	return result
	
func _create_resource_count_node(count: int) -> Label:
	var result := Label.new()
	result.text = str(count)
	return result
		
func load_region(region: RegionObject) -> void:
	if _region_object != null and region.get_region_id() == _region_object.get_region_id():
		return
		
	_clear()
	
	_region_object = region
	_update_region()
	_connect_region_object(region)
	_region_object = region
	
func _update_region() -> void:
	var city_id_opt := _region_object.get_city_id()
	if city_id_opt.is_empty():
		return
		
	# TODO: This will not permit displaying cities of other civs
	var city : City = CurrentGame.get_current_player_civ().find_city_by_id(city_id_opt)
	# TODO: Replace with resources of this region
	var resources : Dictionary = city.build_pnl().total
	
	# TODO: We can actually optimize it and simply increase number in the node,
	# not redraw the whole container
	_clear_widgets()
	for res_id : String in resources:
		add_child(_create_resource_name_node(res_id))
		add_child(_create_resource_count_node(resources[res_id]))
		pass
		
func _on_region_changed(area: Rect2i, flags: int) -> void:
	_update_region()
		
func _connect_region_object(region_object: RegionObject) -> void:
	_region_object.region_changed.connect(_on_region_changed)
	
func _disconnect_region_object(region_object: RegionObject) -> void:
	_region_object.region_changed.disconnect(_on_region_changed)

