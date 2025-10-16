extends Node

class_name GameTile

var _qr_coords : Vector2i
var _offset_coords: Vector2i

var qr_coords:
	get:
		return _qr_coords
		
var offset_coords:
	get:
		return _offset_coords

signal input_event(tile_qr: Vector2i, event: InputEvent)


func _ready() -> void:
	# Get parent
	var surface := get_surface()
	assert(surface != null)
	_offset_coords = (get_parent() as TileMapLayer).local_to_map(self.position)
	_qr_coords = surface.map_to_axial(_offset_coords)
	input_event.connect(surface._on_input_event_from_cell)


## This method relies on the fact that parent TileMapLayer
## is direct descendant from GameTileSurface
func get_surface() -> GameTileSurface:
	var parent := self.get_parent()
	assert(parent != null)
	var p_parent = parent.get_parent() as GameTileSurface
	assert(p_parent != null)
	return p_parent
	

func get_offset_coordinates() -> Vector2i:
	return _offset_coords 

		
func _get_qr_coordinates() -> Vector2i:
	return  _qr_coords
	
func _on_input_event(viewport: Node, event: InputEvent, shape_idx: int) -> void:
	input_event.emit(qr_coords, event)
