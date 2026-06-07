extends GutTest

var conversion_params : Array[Vector2i] = [Vector2i(0,0), Vector2i(1,2), Vector2i(-1,2)]

func test_conversions(coords : Vector2i =use_parameters(conversion_params)) -> void:
	var ref_xy := coords
	
	var qr := QrsCoordsLibrary.xy_to_qr(ref_xy)
	var test_xy := QrsCoordsLibrary.qr_to_xy(qr)
	
	assert_eq(test_xy, ref_xy)
