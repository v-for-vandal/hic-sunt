extends Control

func _ready() -> void:
	pass

func _cancel() -> void:
	self.queue_free()
	
func _save() -> void:
	var save_name : String = $PanelContainer/MainContainer/NewGameContainer/SaveGameName.text
	
	if not CentralSystem.validate_save_name(save_name):
		CentralSystem.error_report("Incorrect save name")
		return
		
	if CentralSystem.has_save(save_name):
		print("Asking for overwrite confirmation")
		# Ask user if he truly wants to override
		var should_overwrite := await CentralSystem.ask_confirm("Overwrite existing save?")
		if not should_overwrite:
			return
		
		
	var success := CentralSystem.save_game(save_name)
	if success != OK:
		CentralSystem.error_report("Failed to save, reason: " + error_string(success) )
		return
		
	# success
	self.queue_free()

func _on_cancel_button_pressed() -> void:
	_cancel()
	
# catch all events to react to ui_cancel and ui_accept
func _input(event: InputEvent) -> void:
	if event.is_action_released("ui_cancel"):
		_cancel()
		accept_event()
		return
	if event.is_action_released("ui_accept"):
		_save()
		accept_event()
		return
		
	return


func _on_save_button_pressed() -> void:
	_save()



func _on_savegame_selected(savegame: String) -> void:
	$PanelContainer/MainContainer/NewGameContainer/SaveGameName.text = savegame


func _on_save_game_name_text_submitted(_new_text: String) -> void:
	# ignore new_text, we will get it anyway
	_save()
