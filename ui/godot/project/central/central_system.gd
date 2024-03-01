extends HicSuntSystem

var _savegames_dir := "user://savegames"

func _ready() -> void:
	pass
	
func validate_save_name(savename: String) -> bool:
	if savename.is_empty():
		return false
		
	if not savename.is_valid_filename():
		return false
		
	return true
	
func _ensure_savegames_dir() -> Error:
	if not DirAccess.dir_exists_absolute(_savegames_dir):
		var status := DirAccess.make_dir_recursive_absolute(_savegames_dir)
		if status != OK:
			push_error("Can't create savegames directory")
	
	return OK
	
func _get_savegames_dir() -> DirAccess:
	if _ensure_savegames_dir() != OK:
		return null
		
	return DirAccess.open(_savegames_dir)
	
func list_saves() -> PackedStringArray:
	var savedir := _get_savegames_dir()
	
	if savedir == null:
		push_error("Can't open savegames directory")
		return []
	savedir.include_hidden = false
	savedir.include_navigational = false
	
	return savedir.get_directories()
	
func has_save(savename: String) -> bool:
	var savedir := _get_savegames_dir()
	
	if savedir == null:
		push_error("Can't open savegames directory")
		return false
		
	return savedir.dir_exists(savename)
	
	
func save_game(savename : String) -> Error:
	var savedir := _get_savegames_dir()
	var status : Error
	
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
	
func load_game(savename : String, ruleset: RulesetObject) -> Error:
	var savedir := _get_savegames_dir()
	var status : Error
	
	if savedir == null:
		push_error("Can't open savegames directory")
		return ERR_DOES_NOT_EXIST
		
	status = savedir.change_dir(savename)
	if status != OK:
		push_error("Cant open directory %s in %s, code: %s" % 
			[savename, ProjectSettings.globalize_path(savedir.get_current_dir()), error_string(status)]
			)
		return ERR_DOES_NOT_EXIST
		
	print("Calling CurrentGame.load_game")
	status = CurrentGame.load_game(savedir, ruleset)
	if status != OK:
		push_error("Failed to load game, status: %s" % [error_string(status)])
		return status
	print("Returning from CentralSystem.load_game")
	return OK
	
func error_report(message : String) -> void:
		var dialog_window := AcceptDialog.new()
		dialog_window.dialog_text = message 
		dialog_window.connect('confirmed', dialog_window.queue_free)
		dialog_window.connect('canceled', dialog_window.queue_free)
		get_tree().current_scene.add_child(dialog_window)
		dialog_window.popup_centered()
	
# This is internal signal for use in ask_confirm
signal _confirm_result(accepted : bool)

func ask_confirm(message: String) -> bool:
	
	var on_confirm := func() -> void:
		_confirm_result.emit(true)
		self.queue_free()
		
	var on_cancel := func() -> void:
		_confirm_result.emit(false)
		self.queue_free()
		
	var dialog_window := ConfirmationDialog.new()
	dialog_window.dialog_text = message
	dialog_window.connect('confirmed', on_confirm)
	dialog_window.connect('canceled', on_cancel)
	get_tree().current_scene.add_child(dialog_window)
	dialog_window.popup_centered()
	var result : bool = await _confirm_result
	return result
	
