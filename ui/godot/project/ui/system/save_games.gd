extends ItemList

signal savegame_selected(savegame: String)
signal savegame_activated(savegame: String)

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	clear()
	
	# get list of saves
	var saves := CentralSystem.list_saves()
	
	for save in saves:
		add_item(save)

func _get_savegame_by_index(index: int) -> String:
	return get_item_text(index)

func _on_item_selected(index: int) -> void:
	savegame_selected.emit(_get_savegame_by_index(index))
	
func get_selected_savegame()-> String:
	var selected_items := get_selected_items()
	if selected_items.is_empty():
		return ""
	return _get_savegame_by_index(selected_items[0])


func _on_item_activated(index: int) -> void:
	savegame_activated.emit(_get_savegame_by_index(index))
