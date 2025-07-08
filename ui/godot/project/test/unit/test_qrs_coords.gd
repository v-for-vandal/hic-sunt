extends GutTest

var conversion_params = [Vector2i(0,0), Vector2i(1,2), Vector2i(-1,2)]

func test_conversions(coords=use_parameters(conversion_params)):
	var ref_xy = coords
	
	var qr = QrsCoordsLibrary.xy_to_qr(ref_xy)
	var test_xy = QrsCoordsLibrary.qr_to_xy(qr)
	
	assert_eq(test_xy, ref_xy)
