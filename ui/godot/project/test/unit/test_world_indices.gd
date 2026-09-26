extends GutTestEnviron

# Tests that World indices are populated on civilisation and city creation

func test_create_civilisation_registers_in_world() -> void:
	var civ_id := &"idx_civ_1"
	var civ := Civilisation.create_civilisation(civ_id)
	assert_not_null(civ, "Expected civilisation to be created")
	var found := CurrentGame.game.world.find_civilisation_by_id(civ_id)
	assert_not_null(found, "Civilisation should be registered in World index")
	assert_eq(found, civ, "World should return the same civilisation instance by id")


func test_create_city_registers_in_world_indices() -> void:
	# Arrange: create a civilisation and pick a valid region and a real CellObject
	var civ_id := &"idx_civ_2"
	var civ := Civilisation.create_civilisation(civ_id)
	assert_not_null(civ)
	var region_id := zero_region.get_region_id()
	var cell := zero_region.get_cell(Vector2i(0, 0))
	assert_not_null(cell)
	assert_true(cell.is_valid())
	assert_eq(cell.get_region_id(), region_id)

	# Act
	var result := civ.create_city(cell)
	assert_true(result.is_ok(), "City creation should succeed")
	var city := result.city
	assert_not_null(city)

	# Determine city id via city API (stored in scope; exposed via git_id())
	var city_id := city.git_id()
	assert_ne(city_id, "", "City id should not be empty")

	# Assert: World indices contain this city by both region id and city id
	var world := CurrentGame.game.world
	var by_region := world.find_city_by_region_id(region_id)
	var by_id := world.find_city_by_id(city_id)
	assert_eq(by_region, city, "City should be indexed by region id in World")
	assert_eq(by_id, city, "City should be indexed by city id in World")
