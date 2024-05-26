extends GutTest

class_name GutTestEnviron

var ruleset : RulesetObject
var world : WorldObject
var zero_region : RegionObject # world.get_region(Vector2i(0,0)) - shortcut

func _load_ruleset() -> RulesetObject:
	var core_ruleset_path := ProjectSettings.globalize_path('res://gamedata/utest')
	var _ruleset_dict : Dictionary = CentralSystem.load_ruleset(core_ruleset_path)
	# TODO: Process loading errors properly
	var _ruleset_object : RulesetObject
	if _ruleset_dict.success:
		print("Successfully loaded core ruleset: ", _ruleset_dict.success)
		_ruleset_object = _ruleset_dict.ruleset
	else:
		print("While loading core ruleset, there were errors: ", _ruleset_dict.errors)
	assert(_ruleset_object != null, "Failed to load ruleset")
	
	return _ruleset_object
	
func _create_world() -> WorldObject:
	var world := WorldObject.create_world(Vector2i(1,1), 10)
	assert_true(world.contains(Vector2i(0,0)))
	# each test should set biomes the way it likes
	return world
	
	
	
func before_all() -> void:
	ruleset = _load_ruleset()
	assert_not_null(ruleset, "Failed to load a test ruleset")

func after_all() -> void:
	ruleset = null
	
func before_each() -> void:
	# create new clean world
	world = _create_world() 
	assert_not_null(world, "Failed to create a world")
	zero_region = world.get_region(Vector2i(0,0))
	assert_not_null(zero_region, "Failed to get (0,0) region")
	CurrentGame.init_game(world, ruleset)
	
# == helper methods for descendants

func do_test_equal_by_serialization(target) -> void:
	assert_true(target is Object, "Can only be used with objects")
	# serialize target
	var serialized = target.serialize_to_variant()
	
	# now, load new object from variant
	var new_object = (target as Object).get_script().new()
	new_object.parse_from_variant(serialized)
	
	# save it again
	var reserialized = new_object.serialize_to_variant()
	
	# compare
	assert_eq(serialized, reserialized, "Expected that two consequent serialization will provide same result")

