extends RefCounted

var load_mod_impl_ := preload("res://system/central/impl/load_mod_impl.gd").new()

var _savegames_dir := "user://savegames"


func ensure_savegames_dir() -> Error:
	if not DirAccess.dir_exists_absolute(_savegames_dir):
		var status := DirAccess.make_dir_recursive_absolute(_savegames_dir)
		if status != OK:
			push_error("Can't create savegames directory")

	return OK


func get_savegames_dir() -> DirAccess:
	if ensure_savegames_dir() != OK:
		return null

	return DirAccess.open(_savegames_dir)


func list_saves() -> PackedStringArray:
	var savedir := get_savegames_dir()

	if savedir == null:
		push_error("Can't open savegames directory")
		return []
	savedir.include_hidden = false
	savedir.include_navigational = false

	return savedir.get_directories()


func has_save(savename: String) -> bool:
	var savedir := get_savegames_dir()

	if savedir == null:
		push_error("Can't open savegames directory")
		return false

	return savedir.dir_exists(savename)


func save_game(savename: String) -> Error:
	var savedir := get_savegames_dir()
	var status: Error

	if savedir == null:
		push_error("Can't open savegames directory")
		return ERR_DOES_NOT_EXIST

	if not savedir.dir_exists(savename):
		status = savedir.make_dir(savename)
		if status != OK:
			push_error("Can't create save location")
			return ERR_DOES_NOT_EXIST

	status = savedir.change_dir(savename)
	if status != OK:
		push_error("Can't create save location")
		return ERR_DOES_NOT_EXIST

	print("Saving game to: ", ProjectSettings.globalize_path(savedir.get_current_dir()))
	CurrentGame.save_game(savedir)
	return OK


func load_game(savename: String, ruleset: RulesetObject) -> Error:
	var savedir := get_savegames_dir()
	var status: Error

	if savedir == null:
		push_error("Can't open savegames directory")
		return ERR_DOES_NOT_EXIST

	status = savedir.change_dir(savename)
	if status != OK:
		push_error(
			"Cant open directory %s in %s, code: %s" %
			[savename, ProjectSettings.globalize_path(savedir.get_current_dir()), error_string(status)],
		)
		return ERR_DOES_NOT_EXIST

	print("Calling CurrentGame.load_game")
	status = CurrentGame.load_game(savedir, ruleset)
	if status != OK:
		push_error("Failed to load game, status: %s" % [error_string(status)])
		return status
	print("Returning from CentralSystem.load_game")
	return OK

# This is internal signal for use in ask_confirm
signal _confirm_result(accepted: bool)


func ask_confirm(message: String, current_scene: Node) -> bool:
	var dialog_window := ConfirmationDialog.new()
	var on_confirm := func() -> void:
		_confirm_result.emit(true)
		dialog_window.queue_free()

	var on_cancel := func() -> void:
		_confirm_result.emit(false)
		dialog_window.queue_free()

	dialog_window.dialog_text = message
	dialog_window.connect('confirmed', on_confirm)
	dialog_window.connect('canceled', on_cancel)
	current_scene.add_child(dialog_window)
	dialog_window.popup_centered()
	var result: bool = await _confirm_result
	return result


func load_mods() -> Array:
	var result := []
	# load everything from our content folder
	result.append_array(load_mod_impl_.load_mods_from_folder('res://content'))
	# load everything from user mods folder
	result.append_array(load_mod_impl_.load_mods_from_folder('user://mods'))

	return result
