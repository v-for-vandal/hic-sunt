@tool
extends EditorPlugin

var plugin
var bottom_control

func _enter_tree() -> void:
	# Initialization of the plugin goes here.
	plugin = preload("res://addons/biome_map_editor/biome_map_editor.gd")
	bottom_control = preload("res://addons/biome_map_editor/editor_ui.tscn").instantiate()
	add_inspector_plugin(plugin)
	add_control_to_bottom_panel(bottom_control, "Biome")


func _exit_tree() -> void:
	# Clean-up of the plugin goes here.
	remove_control_from_bottom_panel(bottom_control)
	bottom_control.queue_free()
	remove_inspector_plugin(plugin)
