extends VBoxContainer

const ADD_KEY = &"add"
const MULT_KEY = &"mult"
const VALUE_KEY = &"value"
const LEVEL_KEY = &"level"

const METADATA_DBG_VIEW_KEY = &"dbg.view"

func _ready() -> void:
	var tree : Tree = %Tree
	tree.set_column_custom_minimum_width(0, 200)
	tree.set_column_custom_minimum_width(1, 70)
	tree.set_column_custom_minimum_width(2, 70)
	
	
	self.set_scope(self._create_debug_scope())
	
	tree.item_selected.connect(_set_debug_display)
	tree.nothing_selected.connect(_reset_debug_display)
	
	
func set_scope(scope: ScopeObject) -> void:
	var data : Dictionary = scope.explain_all()
	data.sort()
	var tree := %Tree
	tree.clear()
	var root : TreeItem = tree.create_item()
	
	for varname : String in data:
		var varitem : TreeItem = tree.create_item(root)
		var display_debug_target := DebugDisplayOptions.DisplayTarget.new()
		display_debug_target.variable = varname
		varitem.set_metadata(0, {
			METADATA_DBG_VIEW_KEY: display_debug_target
		})
		varitem.set_text(0, varname)
		if scope.is_string_variable(varname):
			var varvalue := scope.get_string_value(varname)
			varitem.set_text(1, varvalue)
		else:
			var varvalue := scope.get_numeric_value(varname)
			varitem.set_text(1, "%s" % varvalue)
		varitem.collapsed = true
		var vardata : Dictionary = data[varname]
		for scope_id : String in vardata:
			var scope_item : TreeItem = tree.create_item(varitem)
			var scope_data : Dictionary = vardata[scope_id]
			scope_item.set_text(0, scope_id)
			for modifier : String in scope_data:
				self._add_modifier(varname, modifier, scope_data[modifier], scope_item)
				
			
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
		
