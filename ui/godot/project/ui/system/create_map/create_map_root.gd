extends Control

signal generate_requested(world_size: WBConstants.WorldSize, region_size: int)
signal transition_back()

var _maps : Dictionary
var _processing := false
var _generators : Dictionary[int, WorldBuilderRegistry.WorldGeneratorHandle]
var _ui_elements : Dictionary[int, Control]
var _current_selected : int = -1

var no_options_msg_control : Control 

func _init() -> void:
	pass

func _ready() -> void:
	_prepare_generators()
	_prepare_cellinfo()
	_prepare_highlighters()
	
	$DebugUiEventBus.set_main_interaction(self)
	
	var ruleset := CentralSystem.load_ruleset()
	
	var terrain_mapping := ruleset.get_atlas_render()
	
	%WorldSurface.terrain_mapping = terrain_mapping
	%RegionSurface.visualization_data = terrain_mapping

	
func _prepare_generators() -> void:
	# get all possible values for selected category
	var available_generators := WorldBuilderRegistry.get_world_generators()
	
	for generator_handle : WorldBuilderRegistry.WorldGeneratorHandle in available_generators:
		var _name := generator_handle.name
		
		var this_index : int = %SelectGeneratorButton.item_count
		%SelectGeneratorButton.add_item(_name)
		
		_generators[this_index] = generator_handle
		
	_ui_elements[-1] = %NoOptionsMsg
	no_options_msg_control = %NoOptionsMsg
	
	if len(_generators) > 0 :
		%SelectGeneratorButton.select(0)
		# just in case signal is not working yet
		_on_select_button_item_selected(0)

func _prepare_cellinfo() -> void:
	%CellInfo.set_headers(["data", "value"])
	
func _prepare_highlighters() -> void:
	
	_add_highlighter(null, &"None")
	_add_highlighter(preload("res://resources/highlighters/temperature_highlighter.tres"), &"Temperature")
	_add_highlighter(preload("res://resources/highlighters/precipitation_highlighter.tres"), &"Precipitation")
	_add_highlighter(preload("res://resources/highlighters/height_highlighter.tres"), &"Height")

func _add_highlighter(highlighter: HighlighterInterface, label: StringName) -> void:
		var idx : int = %HighlightSelect.item_count
		%HighlightSelect.add_item(label, idx)
		%HighlightSelect.set_item_metadata(idx,  highlighter)
		
func _create_ui_if_absent(index: int) -> void:
	if index in _ui_elements and _ui_elements[index] != null:
		return
		
	var control := _generators[index].create_ui()
	if control != null:
		_ui_elements[index]  = control
	else:
		_ui_elements[index] = no_options_msg_control
	
	
	return

func _on_select_button_item_selected(index: int) -> void:
	if _current_selected in _ui_elements:
		var _current_control := _ui_elements[_current_selected]
		_current_control.visible = false
		if _current_control.get_parent() != null:
			%GeneratorOptionsScroll.remove_child(_current_control)
			
	_current_selected = index
	_create_ui_if_absent(index)
	assert(index in _ui_elements)
	assert(_ui_elements[index] != null)
	var new_control := _ui_elements[index]
	new_control.visible = true
	%GeneratorOptionsScroll.add_child(new_control)


func clear() -> void:
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

func _do_generate(_debug_mode: bool) -> WorldObject:
	if _current_selected == -1:
		return null
	
	var _selected_generator_module := _generators[_current_selected]
	var _selected_config : Variant = _ui_elements[_current_selected].get_config()
	
	var generator := _selected_generator_module.create_generator(_selected_config)
	
	var result := generator.create_world()
	return result

func _on_generate_button_pressed() -> void:
	if _processing:
		return
	_processing = true
	var world_object := await _do_generate(true)
	%WorldSurface.load_plane(world_object.get_plane(&"main"))
	_processing = false

func _on_map_selected(_name: String, widget: TextureRect) -> void:
	assert( _name.is_empty() or _maps.has(_name))
	if _name.is_empty(): # thats 'none'
		widget.hide()
	else:
		widget.texture = _maps[_name].texture
		widget.show()

func _on_highlight_select_item_selected(index: int) -> void:
	var highlighter := %HighlightSelect.get_item_metadata(index) as HighlighterInterface
	%RegionSurface.highlighter = highlighter
	%WorldSurface.highlighter = highlighter


func _on_foreground_select_item_selected(index: int) -> void:
	var name : String = $%ForegroundSelect.get_item_text(index)
	if index == 0:
		name = ""
		
	_on_map_selected(name, $%DebugForegroundView)


func _on_back_button_pressed() -> void:
	transition_back.emit()


func _on_world_builder_report_progress(message: String, progress: int) -> void:
	print("Received pogress signal ", progress)
	$%GenerationProgressBar.value = progress


func _on_start_game_button_pressed() -> void:
	if _processing:
		return
	_processing = true
	print("Starting game")
	var world_object := await _do_generate(false)
	LoadManager.new_game(world_object)
	_processing = false
	


func _on_select_generator_button_item_selected(index: int) -> void:
	pass # Replace with function body.
	
func on_ui_event(event: GameUiEventBus.UIEvent) -> void:
	if event is UiEventBus.UIMovementEvent:
		if event.prev_qr_coords != event.qr_coords:
			event.surface.clear_select(event.prev_qr_coords)
		event.surface.select(event.qr_coords, true)
		
	if event is UiEventBus.WorldUIMovementEvent:
		var plane : PlaneObject = event.surface.get_plane()
		var region := plane.get_region(event.qr_coords)
		%InfoContainer.set_region(region, event.qr_coords)
		event.accept()
		
	if event is UiEventBus.WorldUIActionEvent:
		if event.action_type == UiEventBus.ActionType.PRIMARY:
			var plane : PlaneObject = event.surface.get_plane()
			var region := plane.get_region(event.qr_coords)
			%RegionSurface.load_region(region)
			
	if event is UiEventBus.RegionUIMovementEvent:
		var region : RegionObject = event.surface.get_region()
		%InfoContainer.set_cell(event.qr_coords)
		event.accept()
		
	
		
	elif event is UiEventBus.CancellationEvent:
		event.accept()
