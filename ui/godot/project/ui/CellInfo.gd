extends GridContainer

func _create_label(text: String) -> Label:
	var result := Label.new()
	result.text = text
	return result
	
func _add_text_label(text: String) -> void:
		add_child(_create_label(text))
	
func _clear() -> void:
	for n : Node in get_children():
		remove_child(n)
		n.queue_free()
	

func _on_region_cell_selected(region: RegionObject, qr: Vector2i) -> void:
	_clear()
	
	if region == null:
		print("Unset world object")
		return
		
	var cell_info : Dictionary = region.get_cell_info(qr)
	if cell_info.is_empty():
		return
		
	
	_add_text_label("QR coords")
	_add_text_label("%d,%d,%d" % [ qr[0], qr[1], 0 - qr[0] - qr[1] ])
	_add_text_label("Terrain")
	_add_text_label(cell_info.terrain)

	
	# show cell properties
	#add_item("QR coords", null, false)
	## add_item(("x %s,%s y" % qr), null, false)
	#add_item("some_text", null, false)
	#add_item("Terrain", null, false)
	#add_item(world_object.get_cell_terrain(qr))
	
