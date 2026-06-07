extends VBoxContainer

var _current_region: RegionObject
var _current_cell: CellObject

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.

## Region is selected when user presses on it (or something like that for console controls)
func on_region_selected(region: RegionObject) -> void:
	# Selecting region  is not the same as loading it. Region is selected when we are
	# in world ui. We should only display informatino about region
	if region != null:
		%ScopeEditor.set_scope(region.get_scope())
	else:
		%ScopeEditor.clear()
		
	_load_region_info(region)
	_current_region = region

## Region is loaded when we open its ui. When region is loaded, we can select cells
func on_region_loaded(region: RegionObject) -> void:
	if region != null:
		%ScopeEditor.set_scope(region.get_scope())
	else:
		%ScopeEditor.clear()
		
	_load_region_info(region)
	_current_region = region
	
func on_region_changed(region: RegionObject) -> void:
	if region == null:
		return
		
	if region.get_id() != _current_region.get_id():
		return
		
	# just reload region
	on_region_loaded(_current_region)
		
# TODO: We should probably handle case when no cell is selected?
func on_cell_selected(cell: CellObject) -> void:
	if cell != null:
		%ScopeEditor.set_scope(cell.get_scope())
		_load_cell_info(cell)
	else:
		if _current_region != null:
			%ScopeEditor.set_scope(_current_region.get_scope())
		else:
			%ScopeEditor.clear()
			
	_current_cell = cell
			
func on_cell_changed(cell: CellObject) -> void:
	if cell == null:
		return
	if _current_cell == null:
		return
		
	if cell.get_id() != _current_cell.get_id():
		return
		
	# just reload interface
	on_cell_selected(_current_cell)
			
func _load_region_info(region : RegionObject) -> void:
	%RegionInfo.clear()
	if region != null:
		var root : TreeItem = %RegionInfo.create_item()
		
		
		var region_info := region.get_info()
		for key : StringName in region_info:
			var item : TreeItem = %RegionInfo.create_item(root)
			item.set_text(0, key)
			item.set_text(1, "%s" % [region_info[key]])
			
func _load_cell_info(cell: CellObject) -> void:
	%CellInfo.clear()
	if cell != null:
		var root : TreeItem = %RegionInfo.create_item()
		
		var cell_improvements : Dictionary[int, ScopeObject] = cell.get_improvements()
		for slot : int in cell_improvements:
			var item : TreeItem = %RegionInfo.create_item(root)
			var scope := cell_improvements[slot]
			item.set_text(0, "imprv_%d" % slot)
			item.set_text(1, scope.GetId())
