# We extend Node, not RefCounted, because it makes way easier to debug
# generators in Godot editor later
extends Node
# TODO: Make abstract in godot 4.5
class_name WorldBuilderGeneratorInterface

## First pass.
## global_context is a context for whole generation process and is shared between
## all modules of all scripts. It can be used for communication between
## modules, but beware that there is no typing.
## Golbal context always contains a few standard items:
## 1. seed - contains some random seed, selected by user
## generator_context is a context that is shared between passes of this generator. It is not
## shared between different modules/generators.
func first_pass(global_context: Dictionary[StringName, Variant],
	generator_context: Dictionary[StringName, Variant]):
		assert(false, "Unimplemented")
		push_error("Unimplemented")
