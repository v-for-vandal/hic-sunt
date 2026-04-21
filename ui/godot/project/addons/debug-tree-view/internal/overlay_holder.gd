extends Control

var _current_overlay := ""

func _ready() -> void:
	self.visible = true
	
func show_overlay(name: String) -> void:
	
	_hide_current_overlay()
	_current_overlay = ""
	if name == "":
		return
		
	var overlay := get_node(name)
	if overlay == null:
		push_error("Can not find overlay with given name %s" % name)
		return
	
	_current_overlay = name
	overlay.visible = true

func _hide_current_overlay() -> void:
	if _current_overlay == "":
		return
	
	var overlay := get_node(_current_overlay)
	if overlay != null:
		overlay.visible = false
