extends VBoxContainer

var open_icon := preload("res://addons/plenticons/icons/64x-hidpi/2d/double-chevron-left-blue.png")
var close_icon := preload("res://addons/plenticons/icons/64x-hidpi/2d/double-chevron-right-blue.png")
var debug_icon := preload("res://addons/plenticons/icons/64x-hidpi/2d/diamond-blue.png")
var build_icon := preload("res://addons/plenticons/icons/64x-hidpi/2d/plus-blue.png")

var _is_open := true

enum OutlinerPosition { Left, Right }

@export
var outliner_position : OutlinerPosition = OutlinerPosition.Right

@onready
var _tabs := [
]

var _active_tab := 0

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	if outliner_position == OutlinerPosition.Left:
		$Control.move_child(%OpenCloseButton, 0)
		$Control/Spacer.visible = false
		%OpenCloseButton.icon = open_icon
	
	if DebugRoot.is_debug_enabled():
		_tabs.append(%DebugTab)
		$Control/TabBar.add_tab("", debug_icon)

	_tabs.append(%BuildingList)
	$Control/TabBar.add_tab("", build_icon)
	
	_tabs[0].visible = true
	
func open_outliner() -> void:
	%OpenCloseButton.icon = close_icon if outliner_position == OutlinerPosition.Right else open_icon
	$Control/TabBar.visible = true
	$ScrollContainer.visible = true
	_is_open = true
	
func close_outliner() -> void:
	%OpenCloseButton.icon = open_icon if outliner_position == OutlinerPosition.Right else close_icon
	_is_open = false
	$Control/TabBar.visible = false
	$ScrollContainer.visible = false

func _on_open_close_button_pressed() -> void:
	if _is_open:
		close_outliner()
	else:
		open_outliner()
		
func on_region_selected(region: RegionObject) -> void:
	%DebugTab.on_region_selected(region)
	# Selecting region  is not the same as loading it. Region is selected when we are
	# in world ui. We should only display informatino about region
	if region != null:
		%BuildingList.load_region(region)
	else:
		%BuildingList.clear()
	
func on_region_loaded(region: RegionObject) -> void:
	%DebugTab.on_region_loaded(region)
	# This method is called when we load region and open region UI
	%BuildingList.load_region(region)
	
func on_region_changed(region: RegionObject) -> void:
	%DebugTab.on_region_changed(region)
	%BuildingList.on_region_changed(region)

func on_cell_changed(cell: CellObject) -> void:
	%DebugTab.on_cell_changed(cell)
	
func on_cell_selected(cell: CellObject) -> void:
	%DebugTab.on_cell_selected(cell)


func _on_tab_bar_tab_changed(tab: int) -> void:
	if tab >= 0 and tab < _tabs.size():
		if _active_tab >= 0 and _active_tab < _tabs.size():
			_tabs[_active_tab].visible = false
			
		_tabs[tab].visible = true
		_active_tab = tab
