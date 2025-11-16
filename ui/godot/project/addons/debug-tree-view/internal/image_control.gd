extends Control

signal set_modulate(color: Color)

func set_texture(texture: Texture2D ) -> void:
	$GridContainer/Size.text = "%d x %d" % [texture.get_width(), texture.get_height()]

func _on_channel_selector_item_selected(index: int) -> void:
	if index == 0:
		set_modulate.emit(Color.WHITE)
		return
	
	var result := Color.BLACK
	# index is shifted by one. '0' is for all-channels, it is handled above
	# red is 1, greend is 2, blue is 3. So subtract one.
	result[index-1] = 1
	
	set_modulate.emit(result)
