extends Control

signal save_requested
signal load_requested

var _load_dialog_scene := preload("res://ui/system/load_game/load_dialog.tscn")

func _on_save_button_pressed() -> void:
	save_requested.emit()


func _on_load_button_pressed() -> void:
	var dialog = _load_dialog_scene.instantiate()
	add_child(dialog)
