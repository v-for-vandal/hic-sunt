@abstract
extends Node2D

class_name GameTileSurface

enum SurfaceType { UNDEFINED, WORLD_SURFACE, REGION_SURFACE}

# must be set in descendant
@export var surface_type : SurfaceType

@export var event_bus : UiEventBus

## Use this variable to get to the DebugTree node that matches this surface
var debug_control : DebugTree.ControlInterface
	

## What to highlight
var highlighter: HighlighterInterface:
	get:
		return highlighter
	set(value):
		highlighter = value
		_display_settings_changed.emit()


# emited when some settings affecting how cells should be displayed are
# changed. This is internal signal, used by scenes in tilemap
signal _display_settings_changed

var _select_layer : TileMapLayer
var _highlight_layer : TileMapLayer

# last time we checked, tile under mouse cursor was this one
var _last_tile_qr := Vector2i(-1000000, -1000000)

var _cube_to_map : Callable
var _map_to_cube : Callable

func _init() -> void:
	debug_control = DebugRoot.get_debug_control().add_text_node("Surface %d" % get_instance_id(), "")

func _ready()->void:
	_select_layer = $select
	_highlight_layer = $highlight
	assert(_select_layer != null, "Can't work without \"select\" layer")
	assert(_highlight_layer != null, "Can't work without \"highlight\" layer")
	
	# init conversion methods
	var conversion_methods := HexagonTileMap.get_conversion_methods_for(
		_select_layer.tile_set.tile_offset_axis,
		_select_layer.tile_set.tile_layout)
	_cube_to_map = conversion_methods.cube_to_map
	_map_to_cube = conversion_methods.map_to_cube
		

@abstract
func _contains(_tile_qr: Vector2i) -> bool;

func _on_input_event_from_cell(tile_qr: Vector2i, event: InputEvent) -> void:
	if _contains(tile_qr):
		if tile_qr != _last_tile_qr:
			var ui_event := _create_movement_event()
			ui_event.prev_qr_coords = _last_tile_qr
			ui_event.qr_coords = tile_qr
			ui_event.surface = self
			event_bus.emit_event(ui_event)
		if event is InputEventMouseButton:
			if event.pressed and not event.is_canceled():
				var ui_event := _create_action_event()
				ui_event.qr_coords = tile_qr
				ui_event.surface = self
				ui_event.action_type = GameUiEventBus.mouse_button_to_action_type(event.button_index)
				event_bus.emit_event(ui_event)
		
		_last_tile_qr = tile_qr

			
func _create_movement_event() -> UiEventBus.UIMovementEvent:
	var ui_event : UiEventBus.UIMovementEvent
	match surface_type:
		SurfaceType.WORLD_SURFACE:
			ui_event = UiEventBus.WorldUIMovementEvent.new()
		SurfaceType.REGION_SURFACE:
			ui_event = UiEventBus.RegionUIMovementEvent.new()
		_: # default
			push_error("surface_type is unset")
			assert(false)
	return ui_event
	
func _create_action_event() -> UiEventBus.UIActionEvent:
	var ui_event : UiEventBus.UIActionEvent
	match surface_type:
		SurfaceType.WORLD_SURFACE:
			ui_event = UiEventBus.WorldUIActionEvent.new()
		SurfaceType.REGION_SURFACE:
			ui_event = UiEventBus.RegionUIActionEvent.new()
		_:
			push_error("surface_type is unset")
	return ui_event
	
func axial_to_map(qr_coords: Vector2i) -> Vector2i:
	var qrs_coords := Vector3i(qr_coords.x, qr_coords.y, - qr_coords.x - qr_coords.y)
	return _cube_to_map.call(qrs_coords)
	
func map_to_axial(xy_coords: Vector2i) -> Vector2i:
	var qrs_coords := _map_to_cube.call(xy_coords) as Vector3i
	return Vector2i(qrs_coords.x, qrs_coords.y)
	
## Overwrite this method if you need to change source id for atlas with selected
## overlays
func _get_select_source_id():
	return 1
	
## Overwrite this method if you need to change source id for atlas with hightlighted
## overlays
func _get_highlight_source_id():
	return _get_select_source_id()
	
func select(qr_coords: Vector2i, good: bool) -> void:
	if !_contains(qr_coords):
		return
	var xy_coords := axial_to_map(qr_coords)
	# TODO: take atlas coords from user settings
	var select_atlas_coords := Vector2i(0,0) if good else Vector2i(1,0)
	_select_layer.set_cell(xy_coords, _get_select_source_id(), select_atlas_coords, 0)
	
func is_selected(qr_coords: Vector2i) -> bool:
	var xy_coords := axial_to_map(qr_coords)
	return _select_layer.get_cell_source_id(xy_coords) != -1
	
	
func clear_select(qr_coords: Vector2i) -> void:
	if !_contains(qr_coords):
		return
	var xy_coords := axial_to_map(qr_coords)
	_select_layer.erase_cell(xy_coords)
	
func clear_all_select() -> void:
	_select_layer.clear()
	
func highlight(qr_coords: Vector2i, good: bool) -> void:
	if !_contains(qr_coords):
		return
	var xy_coords := axial_to_map(qr_coords)
	# TODO: take atlas coords from user settings
	var highlight_atlas_coords := Vector2i(0,0) if good else Vector2i(1,0)
	_highlight_layer.set_cell(xy_coords, _get_highlight_source_id(), highlight_atlas_coords, 0)
	
func clear_highlight(qr_coords: Vector2i) -> void:
	if !_contains(qr_coords):
		return
	var xy_coords := axial_to_map(qr_coords)
	_highlight_layer.erase_cell(xy_coords)
	
func clear_all_highlight() -> void:
	_highlight_layer.clear()
	
func clear() -> void:
	for child in self.get_children():
		if child is TileMapLayer:
			child.clear()
	
