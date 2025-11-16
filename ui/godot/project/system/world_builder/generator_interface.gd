# We extend Node, not RefCounted, because it makes way easier to debug
# generators in Godot editor later
extends Node

# TODO: Make abstract in godot 4.5

## This is an interface for world generator. World generators create worlds.
## There are no limitations on internal structures
class_name WorldGeneratorInterface

func create_world() -> WorldObject:
	assert(false, "Unimplemented")
	@warning_ignore("redundant_await")
	await true
	push_error("Unimplemented")
	return null
