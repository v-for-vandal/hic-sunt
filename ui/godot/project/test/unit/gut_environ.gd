extends GutTest

class_name GutTestEnviron

var ruleset: RulesetObject
var world: World
var plane: WorldPlane
var zero_region: RegionObject # world.get_region(Vector2i(0,0)) - shortcut


func _load_ruleset() -> RulesetObject:
	var core_ruleset_path := ProjectSettings.globalize_path('res://gamedata/core')
	var utest_ruleset_path := ProjectSettings.globalize_path('res://gamedata/utest')
	var all_paths := [core_ruleset_path, utest_ruleset_path]

	var _ruleset_dict: Dictionary = RulesetObject.load(all_paths)
	# TODO: Process loading errors properly
	var _ruleset_object: RulesetObject
	if _ruleset_dict.success:
		print("Successfully loaded core ruleset: ", _ruleset_dict.success)
		_ruleset_object = _ruleset_dict.ruleset
	else:
		print("While loading core ruleset, there were errors: ", _ruleset_dict.errors)
	assert(_ruleset_object != null, "Failed to load ruleset")

	return _ruleset_object



func _create_world() -> World:
	var world_ := World.new()
	var plane_ := world_.create_plane("test_plane", Rect2i(Vector2i(0, 0), Vector2i(2, 2)), 10, 12)
	assert_not_null(plane_)
	assert_true(plane_.plane_object.contains(Vector2i(0, 0)))
	# each test should set biomes the way it likes
	return world_


func before_all() -> void:
	ruleset = _load_ruleset()
	assert_not_null(ruleset, "Failed to load a test ruleset")


func after_all() -> void:
	ruleset = null


func before_each() -> void:
	# create new clean world
	world = _create_world()
	plane = world.get_plane(&"test_plane")
	assert_not_null(world, "Failed to create a world")
	assert_not_null(plane, "Failed to create a plane")
	zero_region = plane.plane_object.get_region(Vector2i(0, 0))
	assert_not_null(zero_region, "Failed to get (0,0) region")
	CurrentGame.init_game(world, ruleset)
	CurrentGame.game.session.create_civilization_scope(&"test_civ")

# == helper methods for descendants


func do_test_equal_by_serialization(target: Object) -> void:
	assert_true(target is Object, "Can only be used with objects")
	# serialize target
	var serialized :Variant = target.serialize_to_variant()

	# now, load new object from variant
	var new_object: Object = (target as Object).get_script().new()
	new_object.parse_from_variant(serialized)

	# save it again
	var reserialized :Variant = new_object.serialize_to_variant()

	# compare
	assert_eq(serialized, reserialized, "Expected that two consequent serialization will provide same result")
