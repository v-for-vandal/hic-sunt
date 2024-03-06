extends GridContainer


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.

func _clear() -> void:
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
	_clear()
	# TODO: Replace with resources of this region
	var resources : Dictionary = region.get_pnl_statement(CurrentGame.get_current_player_ruleset()).get_total()
	
	for res_id : String in resources:
		add_child(_create_resource_name_node(res_id))
		add_child(_create_resource_count_node(resources[res_id]))
		pass
