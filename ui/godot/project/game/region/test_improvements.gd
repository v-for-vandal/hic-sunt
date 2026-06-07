extends GutTestEnviron

func test_create_improvement() -> void:
	var cell := zero_region.get_cell(Vector2i.ZERO)
	assert_not_null(cell)
	if cell == null:
		return
		
	var _IMPROVEMENT_TYPE := &"test.improv.construction_1"
		
	assert_true(cell.is_valid())
	assert_true(CurrentGame.game.session.add_improvement(
		cell,
		&"test_civ",
		_IMPROVEMENT_TYPE
		))
	var improvement := cell.get_improvement(0)
	assert_not_null(improvement, "Should be an improvement at target coords")
	if improvement == null:
		return
	var improvement_type := improvement.get_string_value(WorldConstants.CLASS_VARIABLE)
	assert_eq(improvement_type, _IMPROVEMENT_TYPE,
		"Improvement is not of required class")
