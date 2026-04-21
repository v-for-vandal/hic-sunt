extends GridContainer

signal set_modulate(color: Color)

# Name of an overlay, or "" to disable all
signal overlay_selected(name: String)

func _ready() -> void:
	$OverlaySelector.item_selected.connect(_on_overlay_selected)

func set_texture(texture: Texture2D ) -> void:
	$Size.text = "%d x %d" % [texture.get_width(), texture.get_height()]
	
func set_overlay_holder(overlay_holder: Control) -> void:
	overlay_holder.child_entered_tree.connect(self._on_new_overlay)
	overlay_holder.child_exiting_tree.connect(self._on_remove_overlay)

func _on_channel_selector_item_selected(index: int) -> void:
	if index == 0:
		set_modulate.emit(Color.WHITE)
		return
	
	var result := Color.BLACK
	# index is shifted by one. '0' is for all-channels, it is handled above
	# red is 1, greend is 2, blue is 3. So subtract one.
	result[index-1] = 1
	
	set_modulate.emit(result)
	
func _on_new_overlay(node: Node) -> void:
	$OverlaySelector.add_item(node.name)
	
func _on_remove_overlay(node: Node) -> void:
	# Iterate backwards through items to safely remove while looping
	for i in range($OverlaySelector.get_item_count() - 1, -1, -1):
		if $OverlaySelector.get_item_text(i) == node.name:
			$OverlaySelector.remove_item(i)

func _on_overlay_selected(idx) -> void:
	if idx <= 0:
		overlay_selected.emit("")
	else:
		overlay_selected.emit($OverlaySelector.get_item_text(idx))
		
