const mat2 _pixel_to_pointy_hex = mat2(vec2(sqrt(3.0)/3.0, 0), vec2(-1.0/3.0, 2.0/3.0));

// Convert cube coordinate to horizontal stacked map coordinate.
// [br]See also [method get_conversion_methods_for]
ivec2  axial_to_horizontal_stacked(ivec2 axial_position) {
	int l_x = axial_position.x + ((axial_position.y & ~1) >> 1);
	int l_y = axial_position.y;
	return ivec2(l_x, l_y);
}


// Convert horizontal stacked map coordinate to cube coordinate.
// [br]See also [method get_conversion_methods_for]
 ivec2 horizontal_stacked_to_axial(ivec2 map_position) {
	int l_q = map_position.x - ((map_position.y & ~1) >> 1);
	int l_r = map_position.y;
	return ivec2(l_q, l_r);
}

ivec2 _axial_round(vec2 frac)
{
	float s0 = -frac.x - frac.y;
    float q = round(frac.x);
    float r = round(frac.y);
    float s = round(s0);

    float q_diff = abs(q - frac.x);
    float r_diff = abs(r - frac.y);
    float s_diff = abs(s - s0);

    if (q_diff > r_diff && q_diff > s_diff)
	{
        q = -r-s;
	}
    else if (r_diff > s_diff)
	{
        r = -q-s;
	}
    return ivec2( int( q  ), int( r ));
}

// pixel to hexagon (pointy
ivec2 pixel_to_horizontal_axial(vec2 point, int size) {
	vec2 scaled_point = point / float(size);

	return _axial_round(_pixel_to_pointy_hex * scaled_point);
	//float q = (sqrt(3)/3. * scaled_point.x  -  1./3. * scaled_point.y);
    //float r = (                  2./3. * scaled_point.y);
	//return _axial_round(vec2(q, r));

}

