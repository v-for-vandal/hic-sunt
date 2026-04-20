extends RefCounted

class_name WorldPlane

var _plane_object: PlaneObject
var _substrate : ImageTexture

## Access to plane_object
var plane_object: PlaneObject:
	get:
		assert(_plane_object != null)
		return _plane_object

func _init(plane_object_: PlaneObject) -> void:
	assert(plane_object_ != null)
	_plane_object = plane_object_

func foreach_surface(lambda: Callable) -> void:
	return _plane_object.foreach_surface(lambda)
	
func get_substrate() -> ImageTexture:
	return _substrate
	
func set_substrate(substrate: ImageTexture) -> void:
	_substrate = substrate
	
## Return bounding QR rect describing dimensions of plane in term of regions
func get_qr_dimensions() -> Rect2i:
	return _plane_object.get_dimensions()
	
## Note: Method under consideration
## Returns bounding QR rect describind dimensions of plane in term of cells
## We get it by multiplying dimensions with size of region in term of cells
## For region size we currently use external region radius
func get_cells_qr_dimensions() -> Rect2i:
	var plane_external_radius : int = _plane_object.get_region_external_radius()
	var _plane_dimensions_qr := get_qr_dimensions()
	# 2 * plane_external_radius because it is radius, not diameter
	var _start_qr : Vector2i = _plane_dimensions_qr.position * (2* plane_external_radius) - Vector2i(plane_external_radius, plane_external_radius)
	var _end_qr : Vector2i = _plane_dimensions_qr.end * (2*plane_external_radius)  + Vector2i(plane_external_radius, plane_external_radius)
	var result := Rect2i()
	result.position = _start_qr
	result.end = _end_qr
	
	return result
	
## Convert region-space coordinates to 'global cell coordinate system'. Both are QRS system, region
## is considered having radius 'get_region_external_radius()'
func convert_to_cells_qr(region_qrs_coords: Vector2i) -> Vector2i:
	var plane_external_radius : int = _plane_object.get_region_external_radius()
	return region_qrs_coords * (2 * plane_external_radius)
