extends GutTestEnviron

func test_height() -> void:
	var coords := Vector2i(1,2)
	assert_true(zero_region.contains(coords))
	
	zero_region.set_height(coords, 10.0)
	assert_eq(zero_region.get_height(coords), 10.0)
	
	zero_region.set_height(coords, -10.0)
	assert_eq(zero_region.get_height(coords), -10.0)

	
