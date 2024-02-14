extends ItemList

func _init():
	clear()

func add_building(building_id: String):
	var idx := add_item(building_id)
	set_item_metadata(idx, building_id)
	
