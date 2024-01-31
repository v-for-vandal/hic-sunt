extends Control

signal world_cell_selected(world_object: WorldObject, qr: Vector2i)

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
	
