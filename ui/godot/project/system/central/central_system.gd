extends Node

var CentralSystemImpl := preload("res://system/central/impl/central_system_impl.gd")
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
func save_game(savename: String) -> Error:
	return _pimpl.save_game(savename)


## Load game from specified location.
## TODO: Ruleset should not be input parameter
func load_game(savename: String, ruleset: RulesetObject) -> Error:
	return _pimpl.load_game(savename, ruleset)


func error_report(message: String) -> void:
	var dialog_window := AcceptDialog.new()
	dialog_window.dialog_text = message
	dialog_window.connect('confirmed', dialog_window.queue_free)
	dialog_window.connect('canceled', dialog_window.queue_free)
	get_tree().current_scene.add_child(dialog_window)
	dialog_window.popup_centered()


func ask_confirm(message: String) -> bool:
	return await _pimpl.ask_confirm(message, get_tree().current_scene)


func load_ruleset() -> RulesetObject:
	var core_ruleset_path := ProjectSettings.globalize_path('res://gamedata/v1.0')
	
	# get paths to all mods
	var active_mods := _pimpl.get_active_mods()
	var all_paths := [core_ruleset_path]
	
	for mod_path in  active_mods.keys():
		all_paths.append(ProjectSettings.globalize_path(mod_path))
	
	

	var _ruleset_dict: Dictionary = RulesetObject.load(all_paths)
	# TODO: Process loading errors properly
	var _ruleset_object: RulesetObject
	if _ruleset_dict.success:
		print("Successfully loaded core ruleset: ", _ruleset_dict.success)
		_ruleset_object = _ruleset_dict.ruleset
	else:
		print("While loading core ruleset, there were errors: ", _ruleset_dict.errors)
	assert(_ruleset_object != null, "Failed to load ruleset")

	return _ruleset_object
