extends GridContainer


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func _clear():
	for ch in get_children():
		remove_child(ch)
		ch.queue_free()
		
func _create_resource_name_node(res:Dictionary):
	var result := Label.new()
	result.text = res.id
	return result
	
func _create_resource_count_node(count: int):
	var result := Label.new()
	result.text = str(count)
	return result
		
func load_region(region: RegionObject):
	_clear()
	# TODO: Replace with resources of this region
	var resources = CurrentGame.get_current_player_ruleset().get_all_resources()
	
	for res in resources:
		add_child(_create_resource_name_node(res))
		add_child(_create_resource_count_node(42))
		pass
