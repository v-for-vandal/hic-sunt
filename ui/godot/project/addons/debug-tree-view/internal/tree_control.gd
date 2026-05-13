extends "res://addons/debug-tree-view/internal/control_interface.gd"

var _debug_tree: DebugTree
var _item: TreeItem

const _DraggableCamera := preload("res://addons/debug-tree-view/internal/draggable_camera.tscn")
const _ImageControl := preload("res://addons/debug-tree-view/internal/image_control.tscn")
const _ImageHolder := preload("res://addons/debug-tree-view/internal/image_holder.tscn")
const _OverlayHolder := preload("res://addons/debug-tree-view/internal/overlay_holder.tscn")


func _init(debug_tree: DebugTree, item: TreeItem) -> void:
	assert(debug_tree != null)
	assert(item != null)
	_debug_tree = debug_tree
	_item = item


## Returns a new object of type ControlInterface that points to given item.
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
	var text_receiver := RichTextLabel.new()
	text_receiver.bbcode_enabled = true
	if not text.is_empty():
		text_receiver.add_text(text)
		text_receiver.newline()
	var new_item := _add_element(key, text_receiver)
	# new_item is our text node. We set text_receiver as a sink
	# so all calls to add_text will write there
	_debug_tree._add_text_sink(new_item, text_receiver)

	return _wrap_in_class(new_item)

func add_text(text: String) -> void:
	# if we are text node:
	var text_node := _debug_tree._get_text_sink(_item)
	if text_node == null:
		# We need to create text node, but we also need to map it as our text sink.
		var logs_node := add_text_node("log", text)
		text_node = _debug_tree._get_text_sink(logs_node._item)
		_debug_tree._add_text_sink(_item, text_node)

		return

	text_node.add_text(text)
	text_node.newline()

## Adds an image
## Will place image into scroll container, if needed.
func add_image_node(key: String, image: Image) -> RefCounted:

	var display_container := Control.new()
	display_container.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)

	var image_node := _ImageHolder.instantiate()
	image_node.stretch_mode = TextureRect.STRETCH_KEEP
	image_node.texture = ImageTexture.create_from_image(image)

	var overlay_node := _OverlayHolder.instantiate()
	overlay_node.name = "OverlayHolder"
	overlay_node.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)

	var control_node := _ImageControl.instantiate()
	control_node.set_texture(image_node.texture)
	control_node.set_overlay_holder(overlay_node)
	# I don't understand how anchors/presets work ((
	#control_node.set_anchors_preset(Control.PRESET_TOP_LEFT, true)
	#control_node.set_size(Vector2(300, 100))
	#control_node.position = Vector2.ZERO

	var on_set_modulate = func(color: Color):
		image_node.self_modulate = color

	control_node.set_modulate.connect(on_set_modulate)
	control_node.overlay_selected.connect(overlay_node.show_overlay)

	image_node.add_child(overlay_node)
	display_container.add_child(image_node)
	display_container.add_child(control_node)


	var new_item := _add_element(key, display_container)
	_debug_tree._add_overlay_sink(new_item, overlay_node)

	# recenter image after adding it to tree
	image_node.position = display_container.get_viewport_rect().size / 2

	return _wrap_in_class(new_item)

func add_to_overlay(node: Control) -> void:
		# if we are text node:
	var overlay_node := _debug_tree._get_overlay_sink(_item)
	if overlay_node == null:
		push_error("Trying to add to overlay for node that does not support one")
		return

	# overlays start invisible by default
	node.visible = false
	overlay_node.add_child(node)


func add_empty_node(key: String) -> RefCounted:
	var new_item := _debug_tree._add_element(_item, key, null)

	return _wrap_in_class(new_item)


func add_structured_data_node(key: String, data : Dictionary, time_suffixes: bool = false) -> RefCounted:
	var tree := Tree.new()
	tree.hide_root = true
	tree.columns = 2

	var root_item := tree.create_item()
	root_item.collapsed = false
	_root_fill_structured_data(root_item, data, time_suffixes)
	return _wrap_in_class(
		_add_element(key, tree),
	)



func _root_fill_structured_data(parent: TreeItem, data: Dictionary, time_suffixes: bool) -> void:
	parent.collapsed = false
	for key in data:
		_add_structured_data_value(parent, str(key), data[key], time_suffixes)


func _add_structured_data_value(parent: TreeItem, key: String, value: Variant, time_suffixes: bool) -> void:
	var item := parent.create_child(parent.get_child_count())
	item.set_text(0, DebugTree._normalize_key(key))
	item.collapsed = false

	if value is Dictionary:
		for child_key in value:
			_add_structured_data_value(item, str(child_key), value[child_key], time_suffixes)
		return

	if value is Array:
		for index in value.size():
			_add_structured_data_value(item, str(index), value[index], time_suffixes)
		return

	item.set_text(1, _format_structured_data_value(key, value, time_suffixes))


func _format_structured_data_value(key: String, value: Variant, time_suffixes: bool) -> String:
	if time_suffixes:
		if _get_time_suffix_scale(key) != -1.0:
			var formatted_time := _try_format_time_value(key, value)
			if formatted_time != "":
				return formatted_time

	return str(value)


func _try_format_time_value(key: String, value: Variant) -> String:
	var unit_scale := _get_time_suffix_scale(key)
	if unit_scale < 0.0:
		return ""

	if typeof(value) != TYPE_INT and typeof(value) != TYPE_FLOAT:
		return ""

	var seconds := float(value) * unit_scale
	var abs_seconds := absf(seconds)

	if abs_seconds >= 0.1:
		return "%.3f s" % seconds
	if abs_seconds >= 0.0001:
		return "%.3f ms" % (seconds * 1000.0)
	if abs_seconds >= 0.0000001:
		return "%.3f us" % (seconds * 1000000.0)
	return "%.3f ns" % (seconds * 1000000000.0)


func _get_time_suffix_scale(key: String) -> float:
	if key.ends_with("_ns"):
		return 0.000000001
	if key.ends_with("_us"):
		return 0.000001
	if key.ends_with("_ms"):
		return 0.001
	if key.ends_with("_s"):
		return 1.0
	return -1.0


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
func add_random_group(prefix: String = "") -> RefCounted:
	var key: String = "%s%X" % [prefix, randi()]
	var element := _debug_tree._get_element(_item, key)
	while element != null:
		key = "%s%X" % [prefix, randi()]

	return _wrap_in_class(_add_element(key, null))

func find_node(path: String) -> RefCounted:
	var components := path.split('/', false)
	if components.is_empty():
		return self
	var item := _item
	for component in components:
		var child_item := _debug_tree._get_element(item, component)
		if child_item == null: # no such item
			return null

		assert(child_item != null)
		item = child_item

	return _wrap_in_class(item)
