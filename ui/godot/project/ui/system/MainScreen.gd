extends Control

@export var load_dialog_scene : PackedScene

func _on_new_game_button_pressed() -> void:
	LoadManager.new_game()
	#pass # Replace with function body.


func _on_load_game_button_pressed() -> void:
	var dialog = load_dialog_scene.instantiate()
	add_child(dialog)
