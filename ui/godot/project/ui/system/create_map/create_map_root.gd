extends Control

signal generate_requested(world_size: WBConstants.WorldSize, region_size: int)

var _maps : Dictionary

func clear():
	_maps.clear()
	
	
func add_map(name: String, map: Image, legend: Dictionary) -> void:
	if _maps.has(name):
		# print warning, because generator should not export map with same name
		# more than once
		push_error("map with name ", name, " already exists, overwriting") 
	else:
		# add to selectors
		$PanelContainer/VBoxContainer/ViewControls/BackgroundSelect.add_item(name)
		$PanelContainer/VBoxContainer/ViewControls/ForegroundSelect.add_item(name)
	
	# add to dictionary
	_maps[name] = {
			"texture" : ImageTexture.create_from_image(map),
			"legend" : legend,
			}


func _on_generate_button_pressed() -> void:
	generate_requested.emit(
		$PanelContainer/VBoxContainer/GridContainer/WorldSizeSelector.get_selected_id(),
		int($PanelContainer/VBoxContainer/GridContainer/RegionSizeSelector.value)
	)

func _on_map_selected(name: String, widget: TextureRect) -> void:
	assert( name.is_empty() or _maps.has(name))
	if name.is_empty(): # thats 'none'
		widget.hide()
	else:
		widget.texture = _maps[name].texture
		widget.show()

func _on_background_select_item_selected(index: int) -> void:
	name = $PanelContainer/VBoxContainer/ViewControls/BackgroundSelect.get_item_text(index)
	if index == 0:
		name = ""
		
	_on_map_selected(name, $DisplayContainer/Background)
	
	


func _on_foreground_select_item_selected(index: int) -> void:
	name = $PanelContainer/VBoxContainer/ViewControls/ForegroundSelect.get_item_text(index)
	if index == 0:
		name = ""
		
	_on_map_selected(name, $DisplayContainer/Foreground)
