extends RefCounted

class_name QrsCoordsLibrary

static func qrs_to_xy(qr:Vector2i)->Vector2i :
	var q := qr[0]
	var r := qr[1]
	var col := q  + (r - (r&1)) / 2
	var row := r
	return Vector2i(col, row)
	
	
static func xy_to_qrs(xy: Vector2i)->Vector2i:
	var col := xy[0]
	var row := xy[1]
	var q := col - (row - (row&1)) / 2
	var r := row
	
	return Vector2i(q,r)
