extends Control

@onready var _save_games_list := $PanelContainer/VBoxContainer/SaveGames

func _load(savegame: String) -> void:
	LoadManager.load_game(savegame)
	self.queue_free()
	
func _cancel() -> void:
	self.queue_free()

func _on_load_button_pressed() -> void:
	# See if anything is selected
	var selected_savegame : String = _save_games_list.get_selected_savegame()
	if selected_savegame.is_empty():
		return
		
	_load(selected_savegame)


func _on_save_games_savegame_activated(savegame: String) -> void:
	_load(savegame)


func _on_cancel_button_pressed() -> void:
	_cancel()
