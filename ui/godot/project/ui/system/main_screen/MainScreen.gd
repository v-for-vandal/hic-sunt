extends Control

#@export var load_dialog_scene : PackedScene

var load_dialog_scene := preload("res://ui/system/load_game/load_dialog.tscn")

@onready var _current_scene = $MainMenuRoot

var _history = []
var _load_dialog_scene := preload("res://ui/system/load_game/load_dialog.tscn")

# TODO: RM method
func _init() -> void:
	# test a few things
	# TODO: RM
	print("Test instantiating")
	print( ClassDB.instantiate("String"))
	print( ClassDB.instantiate("Object"))
	print( ClassDB.instantiate("CityProject"))
	
func _ready() -> void:
	# hide everything
	for child in get_children():
		child.hide()
		
	_current_scene.show()
	
	
func _transition(new_scene):
	_current_scene.hide()
	_history.push_back(_current_scene)
	_current_scene = new_scene
	_current_scene.show()
	
func _on_transition_back():
	if _history.is_empty():
		return
	_transition(_history.pop_back())
	
	
func _on_new_game_button_pressed() -> void:
	#LoadManager.new_game()
	#pass # Replace with function body.
	_transition($CreateMapRoot)


func _on_load_game_button_pressed() -> void:
	var dialog = _load_dialog_scene.instantiate()
	add_child(dialog)
	pass
