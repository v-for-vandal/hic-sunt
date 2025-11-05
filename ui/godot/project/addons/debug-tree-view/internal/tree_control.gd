extends "res://addons/debug-tree-view/internal/control_interface.gd"

var _debug_tree: DebugTree
var _item : TreeItem

func _init(debug_tree: DebugTree, item: TreeItem) -> void:
	assert(debug_tree != null)
	assert(item != null)
	_debug_tree = debug_tree
	_item = item
	
func _wrap_in_class(item: TreeItem) -> RefCounted:
	return get_script().new(_debug_tree, item)
	

func add_2d_node(key: String, node: Node2D) -> RefCounted:
	return null
	
	
	
func add_text_node(key: String, text: String) -> RefCounted:
	# create text node
	var text_node := RichTextLabel.new()
	text_node.add_text(text)
	var new_item := _debug_tree._add_element(_item, key, text_node)
	
	return _wrap_in_class(new_item)
	
func add_empty_node(key: String) -> RefCounted:
	var new_item := _debug_tree._add_element(_item, key, null)
	
	return _wrap_in_class(new_item)
	
	
	
func add_group(path: String) -> RefCounted:
	var components := path.split('/', false)
	if components.is_empty():
		return self
	var item := _item
	for component in components:
		var child_item := _debug_tree._get_element(item, component)
		if child_item == null: # no such item
			child_item = _debug_tree._add_element(item, component, null)
		
		assert(child_item != null)
		item = child_item
			
	return _wrap_in_class(item)
