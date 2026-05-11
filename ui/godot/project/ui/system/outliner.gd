extends VBoxContainer

var open_icon := preload("res://addons/plenticons/icons/64x-hidpi/2d/double-chevron-left-blue.png")
var close_icon := preload("res://addons/plenticons/icons/64x-hidpi/2d/double-chevron-right-blue.png")
var _is_open := true

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.
	
func open_outliner() -> void:
	%OpenCloseButton.icon = close_icon
	$Control/TabBar.visible = true
	$PanelContainer.visible = true
	_is_open = true
	
func close_outliner() -> void:
	%OpenCloseButton.icon = open_icon
	_is_open = false
	$Control/TabBar.visible = false
	$PanelContainer.visible = false

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass


func _on_open_close_button_pressed() -> void:
	if _is_open:
		close_outliner()
	else:
		open_outliner()
		
func on_region_selected(region: RegionObject) -> void:
	# Selecting region  is not the same as loading it. Region is selected when we are
	# in world ui. We should only display informatino about region
	if region != null:
		%ScopeEditor.set_scope(region.get_scope())
	else:
		%ScopeEditor.clear()
	
func on_region_loaded(_region: RegionObject) -> void:
	# This method is called when we load region and open region UI
	pass
