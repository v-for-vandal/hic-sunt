extends VBoxContainer

@export var category : WorldBuilderRegistry.CATEGORY = WorldBuilderRegistry.CATEGORY.Heightmap
@export var text: String:
	set(value):
		text = value
		if !is_inside_tree:
			return
		$Selector/Label.text = value

var _ui_elements : Dictionary[int, Control] = {}
var _modules : Dictionary[int, WorldBuilderRegistry.WorldGeneratorModuleHandle] = {}
var _current_selected := -1

func get_selected_module() -> WorldBuilderRegistry.WorldGeneratorModuleHandle:
	return _modules[_current_selected]
	
func get_selected_config() -> Variant:
	if _current_selected in _ui_elements:
		var ui_element : Control = _ui_elements[_current_selected]
		if ui_element != null:
			return ui_element.get_config() 
		else:
			return null
	else:
		return {}

func _init() -> void:
	pass

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	# Init bound child properties
	$Selector/Label.text = text
	
	# get all possible values for selected category
	var available_generators := WorldBuilderRegistry.get_modules_for_category(category)
	
	for generator_handle : WorldBuilderRegistry.WorldGeneratorModuleHandle in available_generators:
		var name := generator_handle.name
		
		var this_index : int = %SelectButton.item_count
		%SelectButton.add_item(name)
		
		_modules[this_index] = generator_handle
		
	if len(_modules) > 0:
		%SelectButton.select(0)
		_on_select_button_item_selected(0)
		
			
func _create_ui_if_absent(index: int) -> void:
	if index in _ui_elements and _ui_elements[index] != null:
		return
		
	var control := _modules[index].create_ui()
	if control != null:
		self.add_child(control)
	_ui_elements[index]  = control
	
	return

func _on_select_button_item_selected(index: int) -> void:
	if _current_selected in _ui_elements:
		_ui_elements[_current_selected].visible = false
	_current_selected = index
	_create_ui_if_absent(index)
	if index in _ui_elements and _ui_elements[index] != null:
		_ui_elements[index].visible = true
		
		
