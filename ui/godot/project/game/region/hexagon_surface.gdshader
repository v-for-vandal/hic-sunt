// Mostly copied from https://godotshaders.com/shader/hexagonal-tilemap-simple-version/

shader_type canvas_item;

#include "res://resources/hexagon/hexagon_functions.gdshaderinc"

group_uniforms Atlas;
//  ****  DATA to pass to this shader  **** ///
// array containing the textures to paint the different types of tiles (eg plain, water,...)
// These textures must have the REPEAT Flag. (Mimpmaps and filter are also good ideas)
// They should be seamless to avoid visual artifacts.
uniform sampler2DArray texture_atlas;



group_uniforms;

group_uniforms MapData;

// This arrays stores data for every cell, in axial coordinates
// in 'red' channel we store index of target biome texture in atlas
// other channels are reserved for now
uniform sampler2D map_data;
uniform ivec2 origin;




// size of an hexa in pixels
uniform int cell_size = 32;
// size of the hexa grid, +1  (eg (101,101) for a 100x100 grid)
//uniform vec2 GridSizePlusOne = vec2(101,101);
// size of the world in pixels: set to CellSize * (1.5f , Sqrt(3)) * GridSize
//uniform vec2 WorldPixelSize;

group_uniforms;

//// ***  Reading texture data for one hexagon  *** ////
// for each hexagon, the texture id is stored in mapdata, in the red chanel.
// Fetch Texture Id from hexa coordinates (in "offset coordinates")
vec4 get_tile_info(sampler2D map,  ivec2 offset_coords)
{
	vec2 float_coords = vec2(offset_coords) / vec2(textureSize(map, 0));
	vec4 data = texture(map,  float_coords, 0);
	return data;
	//return int(color.g * 255.) % texture_atlas_count;
	//return int(color.r * 255.);
}

// Fetch color from at point xy for specified texture id
// input:  xy :  coordinates of the pixel in the world
// tileId : id of the texture to read
vec4 getColorForCurrentPixel(vec2 xy, int tileId)
{
	vec2 atlastxy =  xy / vec2(textureSize(texture_atlas, 0).xy) ;
	return texture( texture_atlas,  vec3( atlastxy.x , atlastxy.y , float(tileId) )  );
}

void fragment()
{
	// pixel position scaled to hexagon size
	vec2 xy = UV * vec2(textureSize(TEXTURE, 0));

	ivec2 axial_coords = pixel_to_horizontal_axial(xy, cell_size);
	ivec2 offset_coords = axial_to_horizontal_stacked(axial_coords) - origin;

	vec4 tile_info = get_tile_info(map_data, offset_coords);

    // blend these colors according to weighting
	//COLOR = getColorForCurrentPixel( xy , tileId );
	//COLOR = texture(mapData, UV);
	//COLOR = texture(textureAtlas, vec3(UV, cos(TIME / 10.) * float(textureAtlaCount)));
	vec2 axial_color = vec2(float(axial_coords.x % 10) / 10.0 , float(axial_coords.y % 10) / 10.0);
	vec2 offset_color = vec2(float(offset_coords.x % 10) / 10.0 , float(offset_coords.y % 10) / 10.0);
	vec2 tile_id_color = vec2( float(tile_info.r  ), 0.5);

	//COLOR = vec4( offset_color , 0.0, tile_info.a);
	//COLOR = vec4(0., 0., 0., 1.);
	//COLOR.b = texture( TEXTURE, vec2(offset_coords) / /*GridSizePlusOne*/ 30., 0).b;
}
