extends VBoxContainer

const ADD_KEY = &"add"
const MULT_KEY = &"mult"
const VALUE_KEY = &"value"
const LEVEL_KEY = &"level"

const METADATA_DBG_VIEW_KEY = &"dbg.view"
const METADATA_IS_VARIABLE_LEAF_KEY = &"dbg.is_variable_leaf"

var _scope: ScopeObject

func _ready() -> void:
	var tree : Tree = %Tree
	tree.set_column_custom_minimum_width(0, 100)
	tree.set_column_expand(0, true)
	tree.set_column_custom_minimum_width(1, 50)
	tree.set_column_custom_minimum_width(2, 50)


	self.set_scope(self._create_debug_scope())

	tree.item_selected.connect(_set_debug_display)
	tree.nothing_selected.connect(_reset_debug_display)

func clear() -> void:
	%Tree.clear()

func set_scope(scope: ScopeObject) -> void:
	assert(scope != null)
	$ScopeName.text = scope.get_id()
	_scope = scope
	var data : Dictionary = scope.explain_all()
	data.sort()
	var tree := %Tree
	tree.clear()
	var root : TreeItem = tree.create_item()
	var path_items := {}

	for varname : String in data:
		var varitem := self._create_variable_path_item(tree, root, path_items, varname)
		var display_debug_target := DebugDisplayOptions.DisplayTarget.new()
		display_debug_target.variable = varname
		varitem.set_metadata(0, {
			METADATA_DBG_VIEW_KEY: display_debug_target,
			METADATA_IS_VARIABLE_LEAF_KEY: true,
		})
		var var_is_string := scope.is_string_variable(varname)
		if var_is_string:
			var varvalue := scope.get_string_value(varname)
			varitem.set_text(1, varvalue)
		else:
			var varvalue := scope.get_numeric_value(varname)
			varitem.set_text(1, "%s" % varvalue)
		varitem.set_text(2, "mturn: %s" % scope.get_modification_time(varname))
		varitem.collapsed = true
		var vardata : Dictionary = data[varname]
		for scope_id : String in vardata:
			var scope_item : TreeItem = tree.create_item(varitem)
			var scope_data : Dictionary = vardata[scope_id]
			var scope_id_text := scope_id if scope_id.length() > 0 else '""'
			scope_item.set_text(0, scope_id_text)
			# Those act as headers
			if var_is_string:
				scope_item.set_text(1, &"level")
				scope_item.set_text(2, &"value")
			else:
				scope_item.set_text(1, &"add")
				scope_item.set_text(2, &"mult")
			for modifier : String in scope_data:
				self._add_modifier(varname, modifier, scope_data[modifier], scope_item)

	_update_parent_button()


func _create_variable_path_item(tree: Tree, root: TreeItem, path_items: Dictionary, varname: String) -> TreeItem:
	var components := self._split_variable_name(varname)
	var current_parent := root
	var current_path := ""

	for component in components:
		current_path = component if current_path.is_empty() else "%s/%s" % [current_path, component]
		var next_item : TreeItem = path_items.get(current_path, null)
		if next_item == null:
			next_item = tree.create_item(current_parent)
			next_item.set_text(0, component)
			path_items[current_path] = next_item
		elif next_item.get_parent() != current_parent:
			next_item = self._move_tree_item_to_parent(tree, next_item, current_parent)
			path_items[current_path] = next_item

		current_parent = next_item

	var full_path := current_path
	if self._is_variable_leaf(current_parent):
		var value_item : TreeItem = path_items.get("%s/$v" % full_path, null)
		if value_item == null:
			value_item = tree.create_item(current_parent)
			value_item.set_text(0, "$v")
			path_items["%s/$v" % full_path] = value_item
		return value_item

	return current_parent


func _split_variable_name(varname: String) -> PackedStringArray:
	var components := PackedStringArray()
	for slash_component in varname.split("/", false):
		for dot_component in slash_component.split(".", false):
			components.append(dot_component)
	return components


func _is_variable_leaf(item: TreeItem) -> bool:
	var metadata_raw : Variant = item.get_metadata(0)
	if metadata_raw == null:
		return false
	var metadata := metadata_raw as Dictionary
	if metadata == null:
		return false
	return metadata.get(METADATA_IS_VARIABLE_LEAF_KEY, false)


func _move_tree_item_to_parent(tree: Tree, item: TreeItem, new_parent: TreeItem) -> TreeItem:
	var moved_item := tree.create_item(new_parent)
	for column in 3:
		moved_item.set_text(column, item.get_text(column))
		moved_item.set_metadata(column, item.get_metadata(column))
		moved_item.set_custom_color(column, item.get_custom_color(column))
		moved_item.set_tooltip_text(column, item.get_tooltip_text(column))
	moved_item.collapsed = item.collapsed
	moved_item.visible = item.visible
	moved_item.disable_folding = item.disable_folding
	moved_item.selectable = item.selectable

	var child := item.get_first_child()
	while child != null:
		var next_child := child.get_next()
		self._move_tree_item_to_parent(tree, child, moved_item)
		child = next_child

	item.free()
	return moved_item


func _add_modifier(varname: String, modifier: String, modifier_data: Dictionary, parent: TreeItem) -> void:
	var tree : Tree = %Tree
	var moditem := tree.create_item(parent)
	var display_debug_target := DebugDisplayOptions.DisplayTarget.new()
	display_debug_target.variable = varname
	display_debug_target.modifier = modifier
	moditem.set_metadata(0, {
		METADATA_DBG_VIEW_KEY: display_debug_target
	})
	moditem.set_text(0, modifier)
	if ADD_KEY in modifier_data:
		moditem.set_text(1, str(modifier_data[ADD_KEY]))
	if VALUE_KEY in modifier_data:
		moditem.set_text(1, str(modifier_data[VALUE_KEY]))
	if MULT_KEY in modifier_data:
		moditem.set_text(2, str(modifier_data[MULT_KEY]))
	if LEVEL_KEY in modifier_data:
		moditem.set_text(2, str(modifier_data[LEVEL_KEY]))

func _reset_debug_display() -> void:
	DebugRoot.get_debug_display_options().target_variable_modifier_on_cell = null

func _set_debug_display() -> void:
	var target_item : TreeItem = %Tree.get_selected()
	if target_item == null:
		_reset_debug_display()
		return

	var metadata_raw : Variant = target_item.get_metadata(0)
	if metadata_raw == null:
		return

	var metadata := metadata_raw as Dictionary
	if metadata == null:
		# do nothing. It could be some intermediary node
		return

	var dbg_target : DebugDisplayOptions.DisplayTarget = metadata.get(METADATA_DBG_VIEW_KEY, null)

	DebugRoot.get_debug_display_options().target_variable_modifier_on_cell = dbg_target

func _create_debug_scope() -> ScopeObject:
	var result := ScopeObject.create_scope()
	result.add_numeric_modifier("variable", "test_modifer1", 0.2, 0.4)
	result.add_numeric_modifier("variable", "test_modifer2", 0.4, 0.6)

	result.add_string_modifier("string_var", "test_string_modifier1", "string_low", 50)
	result.add_string_modifier("string_var", "test_string_modifier1", "string_high", 100)

	return result

func _update_parent_button() -> void:
	if _scope == null:
		return
	var parent_scope := _scope.get_parent()
	if parent_scope == null:
		%ParentButton.set_text("parent: none" )
		%ParentButton.disabled = true
	else:
		%ParentButton.set_text("parent: \"%s\"" % parent_scope.get_id() )
		%ParentButton.disabled = false


func _on_parent_button_pressed() -> void:
	if _scope == null:
		return
	var parent_scope := _scope.get_parent()
	if parent_scope == null:
		return
	self.set_scope(parent_scope)
