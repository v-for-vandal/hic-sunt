extends ItemList

signal savegame_selected(savegame: String)

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	clear()
	
	# get list of saves
	var saves := CentralSystem.list_saves()
	
	for save in saves:
		add_item(save)


func _on_item_selected(index: int) -> void:
	savegame_selected.emit(get_item_text(index))
