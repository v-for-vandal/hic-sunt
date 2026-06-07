extends GutTestEnviron


func test_cell_valid() -> void:
	var cell := zero_region.get_cell(Vector2i.ZERO)
	assert_not_null(cell)
	if cell == null:
		return
		
	assert_true(cell.is_valid())
