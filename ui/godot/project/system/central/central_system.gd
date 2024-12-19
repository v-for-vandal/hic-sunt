extends HicSuntSystem


var CentralSystemImpl := preload("res://system/central/central_system_impl.gd")
var _pimpl := CentralSystemImpl.new()


func _init() -> void:
	_pimpl.load_mods()
	pass
	
func _ready() -> void:
	pass
	
func validate_save_name(savename: String) -> bool:
	if savename.is_empty():
		return false
		
	if not savename.is_valid_filename():
		return false
		
	return true
	

## Returns list of all posible saves to load	
func list_saves() -> PackedStringArray:
	return _pimpl.list_saves()

	
## Checks that save game with this name exists
func has_save(savename: String) -> bool:
	return _pimpl.has_save(savename)

	
## Save current game to specified save 
func save_game(savename : String) -> Error:
	return _pimpl.save_game(savename)

	
## Load game from specified location.
## TODO: Ruleset should not be input parameter
func load_game(savename : String, ruleset: RulesetObject) -> Error:
	return _pimpl.load_game(savename, ruleset)
	
	
func error_report(message : String) -> void:
		var dialog_window := AcceptDialog.new()
		dialog_window.dialog_text = message 
		dialog_window.connect('confirmed', dialog_window.queue_free)
		dialog_window.connect('canceled', dialog_window.queue_free)
		get_tree().current_scene.add_child(dialog_window)
		dialog_window.popup_centered()
	


func ask_confirm(message: String) -> bool:
	return await _pimpl.ask_confirm(message, get_tree().current_scene)

	
# TODO: accept ruleset name/path as argument
# TODO: Find solution for name problem. Can't use 'load_ruleset' because it
# hides native method and will not work
func smart_load_ruleset() -> RulesetObject:
	var core_ruleset_path := ProjectSettings.globalize_path('res://gamedata/v1.0')
	var _ruleset_dict : Dictionary = super.load_ruleset(core_ruleset_path)
	# TODO: Process loading errors properly
	var _ruleset_object : RulesetObject
	if _ruleset_dict.success:
		print("Successfully loaded core ruleset: ", _ruleset_dict.success)
		_ruleset_object = _ruleset_dict.ruleset
	else:
		print("While loading core ruleset, there were errors: ", _ruleset_dict.errors)
	assert(_ruleset_object != null, "Failed to load ruleset")
	
	return _ruleset_object
