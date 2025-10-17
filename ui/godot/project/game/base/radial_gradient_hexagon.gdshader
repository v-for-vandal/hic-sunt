shader_type canvas_item;

// This shader is for this weird Polygon2D UV where you put UV manually
// And it is not normalized. So I put it with proportions 110x128 and centered
// around zero.
// Except it seems to be normalized to texture size, so UV will be from -x to +x, but still
// somewhere around zero.
const vec2 center = vec2(0);

void vertex() {
	// Called for every vertex the material is visible on.
}

void fragment() {
	// Called for every pixel the material is visible on.
	COLOR.a = smoothstep(0.25, 0.75, distance(center,  UV));
	//COLOR.rgb = vec3(1.0, 0.5, 0.0);
}

//void light() {
//	// Called for every pixel for every light affecting the CanvasItem.
//	// Uncomment to replace the default light processing function with this one.
//}
