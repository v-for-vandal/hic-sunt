extends Control

signal generate_requested(world_size: WBConstants.WorldSize, region_size: int)
signal transition_back()

var _maps : Dictionary

func clear():
	_maps.clear()
	
	
func _add_debug_map(name: String, map: Image, legend: Dictionary) -> void:
	if _maps.has(name):
		# print warning, because generator should not export map with same name
		# more than once
		push_error("map with name ", name, " already exists, overwriting") 
	else:
		# add to selectors
		$%BackgroundSelect.add_item(name)
		$%ForegroundSelect.add_item(name)
	
	# add to dictionary
	_maps[name] = {
			"texture" : ImageTexture.create_from_image(map),
			"legend" : legend,
			}


func _on_generate_button_pressed() -> void:
	var selected_world_size = $%WorldSizeSelector.get_selected_id()
	var world_cells_size = Vector2i(20, 20)
	if(selected_world_size == WBConstants.WorldSize.Normal):
		world_cells_size = Vector2i(40, 40)
		
	var selected_region_size := int($%RegionSizeSelector.value)
	
	# get_tree().paused = true doesn't work the way we want it to work
	$WorldBuilder.generate(world_cells_size, selected_region_size )



func _on_map_selected(name: String, widget: TextureRect) -> void:
	assert( name.is_empty() or _maps.has(name))
	if name.is_empty(): # thats 'none'
		widget.hide()
	else:
		widget.texture = _maps[name].texture
		widget.show()

func _on_background_select_item_selected(index: int) -> void:
	var name = $%BackgroundSelect.get_item_text(index)
	if index == 0:
		name = ""
		
	_on_map_selected(name, $%DebugBackgroundView)
	
	


func _on_foreground_select_item_selected(index: int) -> void:
	var name = $%ForegroundSelect.get_item_text(index)
	if index == 0:
		name = ""
		
	_on_map_selected(name, $%DebugForegroundView)


func _on_back_button_pressed() -> void:
	transition_back.emit()


func _on_world_builder_report_progress(message: String, progress: int) -> void:
	print("Received pogress signal ", progress)
	$%GenerationProgressBar.value = progress
