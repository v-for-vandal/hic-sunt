extends RefCounted

## You can check if debug is enabled. This is only useful to avoid doing
## some computationally expensive stuff. If you call ordinary methods
## and debug is not enabled, they will do nothing.
func is_debug_enabled() -> bool:
	return false


## Adds custom Node2D object to debug view, with associated key. Current
## content for key will be replaced. Ownership of the node is taken
## by debug view, so debug view will call queued_free when appropriate.
func add_2d_node(key: String, node: Node2D, hint: Rect2i = Rect2i(0, 0, 100, 100), ui: Control = null) -> RefCounted:
	return null


## Adds text as a node with given key
func add_text_node(key: String, text: String) -> RefCounted:
	return null
	
## Adds text if this node is a text node. If it is not, it will create
## a text node and later invocations will add text to this node
func add_text(text: String) -> void:
	return
	
## Adds an image
## Will place image into scroll container, if needed.
func add_image_node(key: String, image: Image) -> RefCounted:
	return null


## Creates an hierarchy and returns a handle to created node
## add_group("a/b/c") will create
## self
##   -> a
##       -> b
##           -> c
## and will return handle to c
func add_group(path: String) -> RefCounted:
	assert(false, "Unimplemented")
	return RefCounted.new()


## Create a new empty node with random name
func add_random_group() -> RefCounted:
	assert(false, "Unimplemented")
	return RefCounted.new()
