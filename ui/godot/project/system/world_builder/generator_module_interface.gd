# We extend Node, not RefCounted, because it makes way easier to debug
# generators in Godot editor later
extends Node
# TODO: Make abstract in godot 4.5

## This is interface for generator module. Generator modules are small building
## blocks that serve to set up one aspect of a plane - for example, height or
## rivers and so on.
class_name WorldGeneratorModuleInterface

## global_context is a context for whole generation process and is shared between
## all modules of all scripts. It can be used for communication between
## modules, but beware that there is no typing.
## Golbal context always contains a few standard items:
## 1. seed - contains some random seed, selected by user
## generator_context is a context that is shared between passes of this generator. It is not
## shared between different modules/generators.
## 2. region_radius - radius of a region. Not every region will follow this
##    radius, but in general this information can be used for some caluclations
func _init(_global_context: Dictionary[StringName, Variant]) -> void:
	pass

## First pass.
func first_pass() -> void:
		assert(false, "Unimplemented")
		push_error("Unimplemented")
