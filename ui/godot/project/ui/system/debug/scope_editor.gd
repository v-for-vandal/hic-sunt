extends VBoxContainer

const ADD_KEY = &"add"
const MULT_KEY = &"mult"
const VALUE_KEY = &"value"
const LEVEL_KEY = &"level"

func _ready() -> void:
	var tree : Tree = %Tree
	tree.set_column_custom_minimum_width(0, 200)
	tree.set_column_custom_minimum_width(1, 70)
	tree.set_column_custom_minimum_width(2, 70)
	var root := tree.create_item()
	tree.hide_root = true
	var var1  := tree.create_item(root)
	var1.set_text(0, "variable")
	var scope1 := tree.create_item(var1)
	scope1.set_text(0, "scope1")
	var scope2 := tree.create_item(var1)
	scope2.set_text(0, "scope2")
	self._add_modifier("test_modifier1", {ADD_KEY : 0.2, MULT_KEY : 0.4}, scope1)
	self._add_modifier("test_modifier2", {VALUE_KEY: "string1", LEVEL_KEY: 100}, scope1)
	self._add_modifier("test_modifier3", {ADD_KEY: -0.4, MULT_KEY : -0.2 }, scope2)
	self._add_modifier("test_modifier4", {ADD_KEY: 1.2, MULT_KEY: 1.4}, scope2)
	
	
func set_scope(scope: ScopeObject) -> void:
	var data : Dictionary = scope.explain_all()
	data.sort()
	var tree := %Tree
	tree.clear()
	var root : TreeItem = tree.create_item()
	
	for varname : String in data:
		var varitem : TreeItem = tree.create_item(root)
		varitem.set_text(0, varname)
		var vardata : Dictionary = data[varname]
		for scope_id : String in vardata:
			var scope_item : TreeItem = tree.create_item(varitem)
			var scope_data : Dictionary = vardata[scope_id]
			scope_item.set_text(0, scope_id)
			for modifier : String in scope_data:
				self._add_modifier(modifier, scope_data[modifier], scope_item)
				
			
func _add_modifier(modifier: String, modifier_data: Dictionary, parent: TreeItem) -> void:
	var tree : Tree = %Tree
	var moditem := tree.create_item(parent)
	moditem.set_text(0, modifier)
	if ADD_KEY in modifier_data:
		moditem.set_text(1, str(modifier_data[ADD_KEY]))
	if VALUE_KEY in modifier_data:
		moditem.set_text(1, str(modifier_data[VALUE_KEY]))
	if MULT_KEY in modifier_data:
		moditem.set_text(2, str(modifier_data[MULT_KEY]))
	if LEVEL_KEY in modifier_data:
		moditem.set_text(2, str(modifier_data[LEVEL_KEY]))
