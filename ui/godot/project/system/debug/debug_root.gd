extends Node

const _DebugTreeWindow = preload("res://system/debug/debug_tree.tscn")

var _debug_tree: Node

var _debug_display_options := DebugDisplayOptions.new()


func _ready() -> void:
	_debug_tree = _DebugTreeWindow.instantiate()
	_debug_tree.visible = true
	add_child(_debug_tree)

func is_debug_enabled() -> bool:
	# TODO: Find a way to enabled/disable debug at startup and/or
	# at runtime
	return true

func get_debug_control() -> DebugTree.ControlInterface:
	return _debug_tree.get_debug_control()
	
func get_debug_display_options() -> DebugDisplayOptions:
	assert(_debug_display_options != null)
	return _debug_display_options
	
