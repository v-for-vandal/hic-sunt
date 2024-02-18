extends Control

signal world_cell_selected(world_object: WorldObject, qr: Vector2i)
signal next_turn_requested()

var _current_select_callback: Variant
var _is_in_select_mode := false

func _gui_input(event):
	if event.is_action("ui_cancel"):
		if _is_in_select_mode:
			# cancel current callback
			_current_select_callback = null
			_is_in_select_mode = false
			accept_event()
		
		
func _on_options_button_toggled(toggled_on):
	if toggled_on:
		$OptionsDialog.visible = true
	else:
		$OptionsDialog.visible = false


func _on_info_button_toggled(toggled_on):
	if toggled_on:
		$InfoTabContainer.visible = true
	else:
		$InfoTabContainer.visible = false


func on_world_cell_clicked_forward(world_object: WorldObject, qr: Vector2i):
	world_cell_selected.emit(world_object, qr)
	


func _on_next_turn_button_pressed():
	next_turn_requested.emit()
