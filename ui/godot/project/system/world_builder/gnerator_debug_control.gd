extends RefCounted

## Class WorldGeneratorDebugControl provides interface to send custom
## images/nodes/etc for debug view.
class_name WorldGeneratorDebugControl

## Adds custom Node2D object to debug view, with associated key. Current
## content for key will be replaced. Ownership of the node is taken
## by debug view, so debug view will call queued_free when appropriate.
func add_debug_node(key: StringName, node: Node2D) -> void:
	pass
