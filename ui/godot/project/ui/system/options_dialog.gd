extends Control

signal save_requested
signal load_requested


func _on_save_button_pressed() -> void:
	save_requested.emit()
