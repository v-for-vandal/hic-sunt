extends Node

const _DebugTreeWindow = preload("res://system/debug/debug_tree.tscn")

var _debug_tree : Node

func _ready() -> void:
	_debug_tree = _DebugTreeWindow.instantiate()
	_debug_tree.visible = true
	add_child(_debug_tree)
	
func get_debug_control() -> DebugTree.ControlInterface:
	return _debug_tree.get_debug_control()
	
