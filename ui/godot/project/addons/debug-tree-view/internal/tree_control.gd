extends "res://addons/debug-tree-view/internal/control_interface.gd"

var _debug_tree: DebugTree
var _item: TreeItem

const _DraggableCamera := preload("res://addons/debug-tree-view/internal/draggable_camera.tscn")
const _ImageControl := preload("res://addons/debug-tree-view/internal/image_control.tscn")


func _init(debug_tree: DebugTree, item: TreeItem) -> void:
	assert(debug_tree != null)
	assert(item != null)
	_debug_tree = debug_tree
	_item = item


func _wrap_in_class(item: TreeItem) -> RefCounted:
	return get_script().new(_debug_tree, item)


func _add_element(key: String, node: Control) -> TreeItem:
	return _debug_tree._add_element(_item, key, node)


func is_debug_enabled() -> bool:
	# developer note: to disable debug, simply use control that sends everything
	# to /dev/null
	return true


func add_2d_node(key: String, node: Node2D, hint: Rect2i = Rect2i(0, 0, 100, 100), ui: Control = null) -> RefCounted:
	if node == null:
		return add_text_node(key, "Error: Node2D object is null")

	var display_container := SubViewportContainer.new()
	var subviewport := SubViewport.new()
	var camera := _DraggableCamera.instantiate()

	display_container.add_child(subviewport)
	display_container.stretch = true
	if ui != null:
		display_container.add_child(ui)
	subviewport.disable_3d = true
	subviewport.physics_object_picking = true
	subviewport.add_child(node)
	subviewport.add_child(camera)

	# set up camera
	# camera.zoom.x = float(hint.size.x) / subviewport.size.x
	# camera.zoom.y = float(hint.size.y) / subviewport.size.y

	return _wrap_in_class(
		_add_element(key, display_container),
	)


func add_text_node(key: String, text: String) -> RefCounted:
	# create text node
	var text_node := RichTextLabel.new()
	text_node.add_text(text)
	var new_item := _add_element(key, text_node)

	return _wrap_in_class(new_item)
	
## Adds an image
## Will place image into scroll container, if needed.
func add_image_node(key: String, image: Image) -> RefCounted:
	
	var display_container := Control.new()
	display_container.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	
	var image_node := TextureRect.new()
	image_node.stretch_mode = TextureRect.STRETCH_KEEP
	image_node.texture = ImageTexture.create_from_image(image)
	image_node.set_anchors_preset(Control.PRESET_CENTER)
	
	var control_node := _ImageControl.instantiate()
	control_node.set_texture(image_node.texture)
	# I don't understand how anchors/presets work ((
	#control_node.set_anchors_preset(Control.PRESET_TOP_LEFT, true)
	control_node.set_size(Vector2(300, 100))
	control_node.position = Vector2.ZERO
	
	var on_set_modulate = func(color: Color):
		image_node.self_modulate = color
		
	control_node.set_modulate.connect(on_set_modulate)
	
	display_container.add_child(image_node)
	display_container.add_child(control_node)
	
	
	var new_item := _add_element(key, display_container)
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
			child_item = _add_element(component, null)

		assert(child_item != null)
		item = child_item

	return _wrap_in_class(item)


## Create a new empty node with random name
func add_random_group() -> RefCounted:
	var key: String = "%X" % randi()
	var element := _debug_tree._get_element(_item, key)
	while element != null:
		key = "%X" % randi()

	return _wrap_in_class(_add_element(key, null))
