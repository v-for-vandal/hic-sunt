extends HBoxContainer


func _on_button_pressed() -> void:
	$Seed.text = str(randi())
