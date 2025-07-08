extends RefCounted

class_name QrsCoordsLibrary

# function for hex-map
static func qr_to_xy(qr:Vector2i)->Vector2i :
	var q := qr[0]
	var r := qr[1]
	@warning_ignore("integer_division")
	var col := q  + (r - (r&1)) / 2
	var row := r
	return Vector2i(col, row)
	
# function for hex-map
static func xy_to_qr(xy: Vector2i)->Vector2i:
	var col := xy[0]
	var row := xy[1]
	@warning_ignore("integer_division")
	var q := col - (row - (row&1)) / 2
	var r := row
	
	return Vector2i(q,r)
	
static func qr_to_qrs(qr:Vector2i) -> Vector3i:
	return Vector3i(qr.x, qr.y, -(qr.x+qr.y))
	
static func distance_squared_qr(from: Vector2i, to: Vector2i):
	return qr_to_qrs(from).distance_squared_to(qr_to_qrs(to))
	

static func flat_top_qrs_to_pixel(qr: Vector2i) -> Vector2 :
	var x : float = 1.5 * qr.x
	var y : float = 0.866 * qr.x + 1.732 * qr.y
	
	return Vector2(x, y)
