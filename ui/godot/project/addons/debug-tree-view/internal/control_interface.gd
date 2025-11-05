extends RefCounted

## Adds custom Node2D object to debug view, with associated key. Current
## content for key will be replaced. Ownership of the node is taken
## by debug view, so debug view will call queued_free when appropriate.
func add_2d_node(key: String, node: Node2D) -> RefCounted:
	return null
	
## Adds text as a node with given key
func add_text_node(key: String, text: String) -> RefCounted:
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
