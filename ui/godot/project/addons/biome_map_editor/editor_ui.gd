extends Control


func _on_add_biome_button_pressed() -> void:
	var biome_button = Button.new()
	$%BiomeLayout.add_child(biome_button)
	var t0 = min($%T0Control.value, $%T1Control.value)
	var t1 = max($%T0Control.value, $%T1Control.value)
	var p0 = min($%P0Control.value, $%P1Control.value)
	var p1 = max($%P0Control.value, $%P1Control.value)
	biome_button.position = Vector2i(t0, p0)
	biome_button.size = Vector2i(t1-t0, p1-p0)
	biome_button.text = "core.biome.unknown"
	biome_button.show()
	
	print("Biome button added")
	pass # Replace with function body.
