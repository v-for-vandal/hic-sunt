// Mostly copied from https://godotshaders.com/shader/hexagonal-tilemap-simple-version/

shader_type canvas_item;

#include "res://resources/hexagon/hexagon_functions.gdshaderinc"

group_uniforms Atlas;
//  ****  DATA to pass to this shader  **** ///
// array containing the textures to paint the different types of tiles (eg plain, water,...)
// These textures must have the REPEAT Flag. (Mimpmaps and filter are also good ideas)
// They should be seamless to avoid visual artifacts.
uniform sampler2DArray textureAtlas;
uniform int textureAtlasCount;
// size of the textures in the atlas
uniform float textureAtlasDimensions = 512;

group_uniforms;

group_uniforms MapData;
// this texture is used as a 2D array containing the tile type for each tile (in the red chanel)
// tile coordinates are encoded using "offseted" coordinates, ie in the same way as in Godot.Tilemap
// texture size is thus equal to the world grid size.
// It * must *  have flag 0 (No mimmaps, No filter, No repeat.)
uniform sampler2D mapData;




// size of an hexa in pixels
uniform int CellSize = 32;
// size of the hexa grid, +1  (eg (101,101) for a 100x100 grid)
uniform vec2 GridSizePlusOne = vec2(101,101);
// size of the world in pixels: set to CellSize * (1.5f , Sqrt(3)) * GridSize
uniform vec2 WorldPixelSize;

group_uniforms;

// for hexa geometry
const float sqrt3 = 1.73205080757;

//// ***  Reading texture data for one hexagon  *** ////
// for each hexagon, the texture id is stored in mapdata, in the red chanel.
// Fetch Texture Id from hexa coordinates (in "offset coordinates")
int get_tile_id(sampler2D map,  ivec2 offset_coords)
{
	vec4 color = texture(map,  vec2(offset_coords) / /*GridSizePlusOne*/ 30., 0);
	return int(color.g * 255.) % textureAtlasCount;
	//return int(color.r * 255.);
}

// Fetch color from at point xy for specified texture id
// input:  xy :  coordinates of the pixel in the world
// tileId : id of the texture to read
vec4 getColorForCurrentPixel(vec2 xy, int tileId)
{
	vec2 atlastxy =  xy / textureAtlasDimensions ;
	return texture( textureAtlas,  vec3( atlastxy.x , atlastxy.y , float(tileId) )  );
}

void fragment()
{
	// pixel position scaled to hexagon size
	vec2 xy = UV * WorldPixelSize ;

	ivec2 axial_coords = pixel_to_horizontal_axial(xy, CellSize);
	ivec2 offset_coords = axial_to_horizontal_stacked(axial_coords);

	int tile_id = get_tile_id(TEXTURE, offset_coords);

    // blend these colors according to weighting
	//COLOR = getColorForCurrentPixel( xy , tileId );
	//COLOR = texture(mapData, UV);
	//COLOR = texture(textureAtlas, vec3(UV, cos(TIME / 10.) * float(textureAtlaCount)));
	vec2 axial_color = vec2(float(axial_coords.x % 10) / 10.0 , float(axial_coords.y % 10) / 10.0);
	vec2 offset_color = vec2(float(offset_coords.x % 10) / 10.0 , float(offset_coords.y % 10) / 10.0);
	vec2 tile_id_color = vec2( float(tile_id  ), 0.5);

	COLOR = vec4( tile_id_color , 0.0, 1.0);
	//COLOR = vec4(0., 0., 0., 1.);
	//COLOR.b = texture( TEXTURE, vec2(offset_coords) / /*GridSizePlusOne*/ 30., 0).b;
}
