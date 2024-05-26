extends GutTestEnviron

func test_creation() -> void:
	var coords := Vector2i(1,2)
	assert_true(zero_region.contains(coords))
	var result = ConstructionProject.create_construction_project(
		&"test.improv.construction_1",
		zero_region, coords)
	assert_not_null(result, "Failed to create construction project")
	
	assert_eq(result.get_improvement_id(), &"test.improv.construction_1")
	assert_true(result.is_possible())
	assert_false(result.is_finished())
	assert_eq(result._region_coords, coords )
	
func test_serialization() -> void:
	var coords := Vector2i(1,2)
	assert_true(zero_region.contains(coords))
	var target = ConstructionProject.create_construction_project(
		&"test.improv.construction_1",
		zero_region, coords)
	assert_not_null(target, "Failed to create construction project")
	do_test_equal_by_serialization(target)
	
func test_creation_failure_out_of_bounds() -> void:
	var coords := Vector2i(1000, 1000)
	assert_false(zero_region.contains(coords))
	var result = ConstructionProject.create_construction_project(
		&"test.improv.construction_1",
		zero_region, coords)
	assert_null(result, "Creating project in coords not present in region must have failed")
	
func test_creation_failure_invalid_improvement_id() -> void:
	var coords := Vector2i(0, 0)
	assert_true(zero_region.contains(coords))
	var result = ConstructionProject.create_construction_project(
		&"nonexistent_id",
		zero_region, coords)
	assert_null(result, "Creating project for non-existend improvement id should have failed")

func test_progress() -> void:
	var coords := Vector2i(2,1)
	assert_true(zero_region.contains(coords))
	assert_true(zero_region.get_cell_info(coords).improvement.is_empty(), "Should be no improvement at target coords")
	var project = ConstructionProject.create_construction_project(
		&"test.improv.construction_1",
		zero_region, coords)
	assert_not_null(project, "Failed to create construction project")
	assert_eq(project.progress_estimate().progress, 0, "Progress should be 0%")
	assert_true(project.is_possible())
	assert_false(project.is_finished())
	
	# total required - 10 wood, 10 stone, 2 workforce for assembly
	
	# Step one
	var resources := {
		&"core.res.stone" : 5,
		&"core.res.wood" : 5,
		&"core.res.workforce" : 10
	}
	# sanity check
	assert_eq(ResourceEconomyLibrary.total_amount(resources), 20)
	
	var status = project.take_resources(resources)
	assert_eq(ResourceEconomyLibrary.total_amount(resources), 0, "All resources should have been taken")
	assert_true(status, "We should have made progress on this step")
	assert_eq(project.progress_estimate().turns_without_progress, 0, "We made progress this turn, should have 0 here")
	assert_between(project.progress_estimate().progress, 40, 50, "Progress should be approximatelly 50%")
	assert_true(project.is_possible())
	assert_false(project.is_finished())
	
	# Step two - no workforce, no changes
	resources = {
		&"core.res.stone" : 5,
		&"core.res.wood" : 5,
		&"core.res.workforce" : 0
	}
	
	status = project.take_resources(resources)
	assert_false(status, "No progress should have been made this turn")
	assert_eq(ResourceEconomyLibrary.total_amount(resources), 10, "No resources should have been taken")
	assert_eq(project.progress_estimate().turns_without_progress, 1, "We made no progress this turn, should have 1 here")
	assert_between(project.progress_estimate().progress, 40, 50, "Progress should be approximatelly 50%")
	assert_true(project.is_possible())
	assert_false(project.is_finished())
	
	# step three - some resources, but not enough workforce
	resources = {
		&"core.res.stone" : 20,
		&"core.res.wood" : 20,
		&"core.res.workforce" : 5
	}
	assert_eq(ResourceEconomyLibrary.total_amount(resources), 45, "Sanity check")
	status = project.take_resources(resources)
	assert_true(status, "We should have made progress this turn")
	assert_eq(ResourceEconomyLibrary.total_amount(resources), 35, "We should have taken only 5 of resource + 5 of workforce")
	assert_true(project.is_possible())
	assert_false(project.is_finished())
	assert_eq(project.progress_estimate().turns_without_progress, 0, "We made progress this turn, should have 0 here")
	
	# step 4 - remaining resources
	resources = {
		&"core.res.stone" : 20,
		&"core.res.wood" : 20,
		&"core.res.workforce" : 100
	}
	status = project.take_resources(resources)
	assert_true(status, "We should have made progress this turn")
	assert_true(project.is_finished(), "Project should have been finished")
	assert_eq(project.progress_estimate().progress, 100, "Progress should be 100%")
	project.execute_finisher()
	assert_false(zero_region.get_cell_info(coords).improvement.is_empty(), "Should be an improvement at target coords")
	assert_true(zero_region.get_cell_info(coords).improvement.type == &"test.improv.construction_1",
		"Finished project should have set an improvement on target cell")
