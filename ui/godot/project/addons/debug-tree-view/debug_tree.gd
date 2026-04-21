extends Control

class_name DebugTree

const _NODE_KEY := &"$node"
const _TEXT_SINK_KEY := &"$text"
const _OVERLAY_SINK_KEY := &"$overlay"

var _current_node: Control = null

## Control interface for debug tree
const ControlInterface = preload("res://addons/debug-tree-view/internal/control_interface.gd")


class TreeControl:
	extends "res://addons/debug-tree-view/internal/tree_control.gd"


func _ready() -> void:
	var root: TreeItem = %Tree.create_item(null)
	__setup_tree_item(root, "<invisible>", null) # root is invisible anyway


## Returns root for this debug tree. This handler is used to populate tree with elements
func get_debug_control() -> ControlInterface:
	return TreeControl.new(self, %Tree.get_root())
	

## This function cretaes simple rectangular element to be used as an overlay.
static func create_rect_overlay(zone : Rect2i, color : Color, name: String) -> Control:
	var display_rect_node := ReferenceRect.new()
	display_rect_node.position = zone.position
	display_rect_node.size = zone.size
	display_rect_node.border_color = color
	display_rect_node.editor_only = false
	display_rect_node.name = name
	display_rect_node.focus_mode = Control.FOCUS_NONE
	display_rect_node.mouse_filter = Control.MOUSE_FILTER_IGNORE
	return display_rect_node


static func _normalize_key(key: String) -> String:
	if key.is_empty():
		return "<unnamed>"
	return String(key.replace("$", ".")) # dollar sign is reserved


static func _get_tree_item_metadata(item: TreeItem) -> Dictionary[String, Variant]:
	var metadata_raw := item.get_metadata(0)
	if metadata_raw == null:
		push_error("Incorrectly setup tree item: metadata is null")
		return { }
	if not (metadata_raw is Dictionary[String, Variant]):
		push_error("Incorrectly setup tree item: metadata is not dictionary")
		return { }
	return metadata_raw as Dictionary[String, Variant]


func _add_element(parent: TreeItem, key: String, node: Control) -> TreeItem:
	key = _normalize_key(key)
	var metadata := _get_tree_item_metadata(parent)

	if key in metadata:
		# change name until we find key that is not present
		var suffix := 2
		while true:
			var key_s := "%s (%s)".format([key, str(suffix)])
			if key_s not in metadata:
				key = key_s
				break

			suffix += 1

	assert(key not in metadata)
	var child_id: int = parent.get_child_count()
	var item := parent.create_child(child_id)
	metadata[key] = child_id
	if node != null:
		node.visible = false
		%ViewContainer.add_child(node)

	# set up item
	__setup_tree_item(item, key, node)

	return item


## Returns item associated with this key in parent, or null if no such
## item is present
func _get_element(parent: TreeItem, key: String) -> TreeItem:
	key = _normalize_key(key)
	var metadata := _get_tree_item_metadata(parent)
	if key in metadata:
		return parent.get_child(metadata[key])

	return null


func __setup_tree_item(item: TreeItem, key: String, node: Control) -> void:
	var metadata: Dictionary[String, Variant] = {
		_NODE_KEY: node,
	}
	item.set_metadata(0, metadata)
	item.set_text(0, key)

func _add_text_sink(item: TreeItem, sink: RichTextLabel) -> void:
	var metadata := _get_tree_item_metadata(item)
	if _TEXT_SINK_KEY in metadata:
		push_error("Attempt to change text sink for a node")
		return
		
	metadata[_TEXT_SINK_KEY] = sink
	
func _get_text_sink(item: TreeItem) -> RichTextLabel:
	var metadata := _get_tree_item_metadata(item)
	return metadata.get(_TEXT_SINK_KEY, null)
	
func _add_overlay_sink(item: TreeItem, sink: Control) -> void:
	var metadata := _get_tree_item_metadata(item)
	if _OVERLAY_SINK_KEY in metadata:
		push_error("Attempt to change overlay sink for a node")
		return
		
	metadata[_OVERLAY_SINK_KEY] = sink
	
func _get_overlay_sink(item: TreeItem) -> Control:
	var metadata := _get_tree_item_metadata(item)
	return metadata.get(_OVERLAY_SINK_KEY, null)

	
func _on_tree_item_selected() -> void:
	var current_item: TreeItem = %Tree.get_selected()
	if current_item == null:
		return

	if _current_node != null:
		_current_node.visible = false
		_current_node = null

	var metadata := _get_tree_item_metadata(current_item)
	var node := metadata.get(_NODE_KEY) as Control
	if node != null:
		node.visible = true

	_current_node = node
