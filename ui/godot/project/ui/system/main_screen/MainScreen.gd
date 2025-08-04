extends Control

@onready var _current_scene = $MainMenuRoot

var _history :Array[Control]= []
const  _load_dialog_scene := preload("res://ui/system/load_game/load_dialog.tscn")
const  _create_map_scene := preload("res://ui/system/create_map/create_map_root.tscn")

const EPHIMERAL_MARKER := &'hicsunt_ephimeral'

# TODO: RM method
func _init() -> void:
	pass
	
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
	
func _transition_back():
	# will return null if history is empty
	var new_scene : Control = _history.pop_back()
	if new_scene == null:
		return
	
	var old_scene = _current_scene
	old_scene.hide()
	new_scene.show()
	
	if old_scene.has_meta(EPHIMERAL_MARKER):
		old_scene.queue_free()
	
func _on_transition_back():
	_transition_back()
	
	
func _on_new_game_button_pressed() -> void:
	# create new scene.
	var create_map_root := _create_map_scene.instantiate()
	create_map_root.hide()
	# add it as child
	add_child(create_map_root)
	# mark as ephimeral
	create_map_root.set_meta(EPHIMERAL_MARKER, true)
	# transition to new schene
	_transition(create_map_root)


func _on_load_game_button_pressed() -> void:
	var dialog := _load_dialog_scene.instantiate()
	add_child(dialog)
	pass
