extends GutTestEnviron

const REGION_MAP_SCENE := preload("res://game/region/region_map.tscn")

func test_region_map_and_ui_start_correctly() -> void:
	var region_map := REGION_MAP_SCENE.instantiate()
	assert_not_null(region_map, "RegionMap scene should instantiate")
	add_child_autofree(region_map)
	await get_tree().process_frame

	assert_eq(region_map.name, "RegionMap")
	assert_true(region_map is Node2D, "RegionMap root should be a Node2D")

	var region_surface := region_map.get_node_or_null("RegionSurface")
	assert_not_null(region_surface, "RegionMap should contain RegionSurface")
	assert_true(region_surface is GameTileSurface, "RegionSurface should be a GameTileSurface")

	var region_ui := region_map.get_node_or_null("CanvasLayer/RegionUI")
	assert_not_null(region_ui, "RegionMap should contain RegionUI")
	assert_true(region_ui is Control, "RegionUI should be a Control")

	region_map.set_event_bus(CurrentGame.event_bus)
	assert_eq(region_surface.event_bus, CurrentGame.event_bus, "RegionSurface should use CurrentGame event bus")

	region_map.load_region(zero_region)
	await get_tree().process_frame

	assert_eq(region_map._region_object, zero_region, "RegionMap should store the loaded region")
	assert_eq(region_ui._region, zero_region, "RegionUI should receive the loaded region")
	assert_not_null(region_ui.get_node_or_null("Outliner"), "RegionUI should have its Outliner")
	assert_not_null(region_ui.get_node_or_null("PanelContainer/HBoxContainer/CloseButton"), "RegionUI should have a close button")
