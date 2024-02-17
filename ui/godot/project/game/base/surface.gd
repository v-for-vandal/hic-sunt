extends TileMap

class_name GameTileSurface

enum SurfaceType { WORLD_SURFACE, REGION_SURFACE}

# must be set in descendant
var surface_type : SurfaceType
var _highlight_layer_id : int = 1

# last time we checked, tile under mouse cursor was this one
var _last_tile_qr := Vector2i(0,0)

func _ready()->void:
	# find highlight layer and remember it
	for i in range(get_layers_count()):
		if get_layer_name(i) == "highlight":
			_highlight_layer_id = i
			break

func _contains(tile_qr: Vector2i):
	printerr("Function must be overriden in child class")
	return false
	
func _unhandled_input(event) -> void:
	if event is InputEventMouse:
		event = make_input_local(event)
		var tile_xy := local_to_map(to_local(event.position))
		var tile_qr := QrsCoordsLibrary.xy_to_qrs(tile_xy)
		if _contains(tile_qr):
			if tile_qr != _last_tile_qr:
				var ui_event = GameUiEventBus.WorldUIMovementEvent.new() if surface_type == SurfaceType.WORLD_SURFACE else GameUiEventBus.RegionUIMovementEvent.new()
				ui_event.prev_qr_coords = _last_tile_qr
				ui_event.qr_coords = tile_qr
				ui_event.surface = self
				GameUiEventBus.emit_event(ui_event)
			if event is InputEventMouseButton:
				if not event.pressed and not event.canceled:
					print("WorldSurface: Cell coords xy:", tile_xy, " qr:", tile_qr)
					var ui_event = GameUiEventBus.WorldUIActionEvent.new() if surface_type == SurfaceType.WORLD_SURFACE else GameUiEventBus.RegionUIActionEvent.new()
					ui_event.qr_coords = tile_qr
					ui_event.surface = self
					ui_event.action_type = GameUiEventBus.mouse_button_to_action_type(event.button_index)
					GameUiEventBus.emit_event(ui_event)
			_last_tile_qr = tile_qr
	
func highlight(qr_coords: Vector2i, good: bool) -> void:
	if !_contains(qr_coords):
		return
	var xy_coords = QrsCoordsLibrary.qrs_to_xy(qr_coords)
	# TODO: take atlas coords from user settings
	var highlight_atlas_coords = Vector2i(0,0) if good else Vector2i(1,0)
	set_cell(_highlight_layer_id, xy_coords, 1, highlight_atlas_coords, 0)
	
func clear_highlight(qr_coords: Vector2i) -> void:
	if !_contains(qr_coords):
		return
	var xy_coords = QrsCoordsLibrary.qrs_to_xy(qr_coords)
	erase_cell(_highlight_layer_id, xy_coords)
	
func clear_all_highlight() -> void:
	clear_layer(_highlight_layer_id)
