extends Control


func _on_options_button_toggled(toggled_on):
	if toggled_on:
		$OptionsDialog.visible = true
	else:
		$OptionsDialog.visible = false
