extends GridContainer

func _create_label(text: String) -> Label:
	var result = Label.new()
	result.text = text
	return result
	
func _add_text_label(text: String):
		add_child(_create_label(text))
	
func _clear():
	for n in get_children():
		remove_child(n)
		n.queue_free()
	

func _on_world_cell_selected(world_object: WorldObject, qr: Vector2i):
	_clear()
	
	if world_object == null:
		print("Unset world object")
		return
	
	_add_text_label("QR coords")

	_add_text_label("%d,%d,%d" % [ qr[0], qr[1], 0 - qr[0] - qr[1] ])
	_add_text_label("Terrain")
	_add_text_label(world_object.get_cell_terrain(qr))

	
	# show cell properties
	#add_item("QR coords", null, false)
	## add_item(("x %s,%s y" % qr), null, false)
	#add_item("some_text", null, false)
	#add_item("Terrain", null, false)
	#add_item(world_object.get_cell_terrain(qr))
	
