extends HBoxContainer

@export var category : WorldBuilderRegistry.CATEGORY = WorldBuilderRegistry.CATEGORY.Heightmap
@export var text: String:
	set(value):
		text = value
		if !is_inside_tree:
			return
		$Selector/Label.text = value

var _ui_elements : Dictionary = {}
var _modules : Dictionary = {}
var _current_selected := -1

func get_selected_module() -> WorldBuilderRegistry.ModuleInfo:
	return _modules[_current_selected]
	
func get_selected_module_config() -> Dictionary:
	if _current_selected in _ui_elements:
		var ui_element : Control = _ui_elements[_current_selected]
		return ui_element.get_config() 
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
	
	for generator_info : WorldBuilderRegistry.ModuleInfo in available_generators:
		var name := generator_info.name
		var generator : RefCounted = generator_info.create_instance()
		
		var this_index : int = $Root/Selector/SelectButton.items_count
		$Root/Selector/SelectButton.add_item(name)
		
		_modules[this_index] = generator_info
		
		if generator.has_method("get_heightmap_ui"):
			var ui_element : Control = generator.get_heightmap_ui()
			$Root.Selector.add_child(ui_element)
			ui_element.shown = false
			
			_ui_elements[this_index] = ui_element
			

func _on_select_button_item_selected(index: int) -> void:
	if _current_selected in _ui_elements:
		_ui_elements[_current_selected].shown = false
	_current_selected = index
	if index in _ui_elements:
		_ui_elements[index].shown = true
