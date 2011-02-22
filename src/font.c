// font.c

#include "common.h"
#include "font.h"
//-----------------------
#include "maths.h"
#include "external/util.h"
#include "render/debugdraw.h"
#include "system/file.h"

// *** The stb TrueType library
#define STB_TRUETYPE_IMPLEMENTATION
#include  "3rdparty/stbTrueType/stb_truetype.h"

unsigned char ttf_buffer[1<<25];
  
#define CHAR_COUNT 256

vglTexture g_glyph[CHAR_COUNT];
vglTexture g_atlas;
vector g_glyphData[CHAR_COUNT];

#define FONT_PATH "assets/font/DroidSansMono.ttf"

// Test init function, based on the main func in the complete program implementation of stb TrueType ((C) Sean Barrett 2009)
void font_init() {
	vfont_loadTTF( FONT_PATH );
}

// Load a TrueTypeFont into the font engine
// This allocates texture(s) for the glyphs
// And computes the glyphs
void vfont_loadTTF( String path ) {
	// *** load the ttf file
	FILE* fontfile = vfile_open( path, "rb" );
	int ret = fread(ttf_buffer, 1, 1<<25, fontfile);
	(void)ret;

	// *** init the tt system and extract font data
	stbtt_fontinfo font;
	stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

#define ATLAS_W 256
#define ATLAS_H 256
#define ATLAS_ROWS 12
#define MAX_ROW_HEIGHT 20
	ubyte font_atlas[ATLAS_H][ATLAS_W];		// Image to atlas all glyph textures into
	u32 row_height[ATLAS_ROWS];				// height of glyph row
	u32 row_width[ATLAS_ROWS];				// Current width of glyph row
	u32 row_start[ATLAS_ROWS];				// tex-Y coordinate each glyph row starts
	memset( font_atlas, 0, ATLAS_H * ATLAS_W );
	memset( row_height, 0, ATLAS_ROWS * 4 );
	memset( row_width, 0, ATLAS_ROWS * 4 );
	memset( row_start, 0, ATLAS_ROWS * 4 );
	
	row_height[0] = 20;
	row_start[0] = 0;
	row_height[1] = 16;
	row_start[1] = 20;
	row_height[2] = 8;
	row_start[2] = 36;

	for (unsigned char ch = 0; ch < 255; ch++) {
		// *** extract a bitmap
		int w,h,scale;
		scale = 20;
		unsigned char *bitmap;
		bitmap = stbtt_GetCodepointBitmap( &font, 0,stbtt_ScaleForPixelHeight(&font, scale), ch, &w, &h, 0, 0 );

		//printf( "Looking to atlas glyph - h %d, w %d\n", h, w );
		assert( h < MAX_ROW_HEIGHT );

		// *** find somewhere to put it in the atlas
		int best = -1;
		u8 best_height = 255;
		for ( int r = 0; r < ATLAS_ROWS; r++ ) {
			//printf( "Attempting row: %d. H: %d, W: %d, besth: %d\n", r, row_height[r], row_width[r], best_height );
			if (( row_height[r] >= h ) && ( ( ATLAS_W - row_width[r] ) >= w ) && ( row_height[r] < best_height )) {
				// put it in this row
				best_height = row_height[r];
				best = r;
			}
			if ( ( row_height[r] == 0 ) && ( best == -1 ) ) {
				// empty row, make a new one
				row_height[r] = (h <= 8) then 8 otherwise ((h <= 16) then 16 otherwise MAX_ROW_HEIGHT); // Take the lowest of 8,12,16 that is big enough
				row_start[r] = (r > 0) ? row_start[r - 1] + row_height[r - 1] : 0;
				assert( row_start[r] != 0 || r == 0 );
				assert( row_start[r] < ATLAS_H );
				best = r;
				break;
			}
			//printf( "best row: %d.\n", best );
		}
		// *** Put it in best
		assert( best != -1 );
		//printf( "Using best row: %d. \n", best );
		// Copy by rows
		for ( int y = 0; y < h; y++ ) {
			memcpy( &font_atlas[ row_start[best] + y ][ row_width[best] ], (bitmap + y * w), w );
		}
		g_glyphData[ch] = Vector( (float)row_width[best]/256.f, (float)row_start[best]/256.f, (float)(row_width[best] + w)/256.f, (float)(row_start[best] + h)/256.f );
		printf( "Glyph: %c : %.2f, %.2f, %.2f, %.2f\n", ch, g_glyphData[ch].coord.x, g_glyphData[ch].coord.y, g_glyphData[ch].coord.z, g_glyphData[ch].coord.w);
		row_width[best] += w;


		g_atlas = vgl_buildTexture( (ubyte*)font_atlas, ATLAS_W, ATLAS_H, VGL_LUMINANCE, VGL_LUMINANCE );




		unsigned char img[16][16];
		memset( img, 0, 16*16 );
		for ( int i = 0 ; (i < h) && (i < 16); i++ ) {
			memcpy( img[i], bitmap + (w * i), w );
		}

		g_glyph[ch] = vgl_buildTexture( (ubyte*)img, 16, 16, VGL_LUMINANCE, VGL_LUMINANCE );

		free( bitmap );
	}

	// Validate row heights + starts
	for (int r = 1; r < ATLAS_ROWS; r ++) {
		assert( (row_start[r] == row_start[r - 1] + row_height[r - 1]) || ( row_start[r] == 0 && row_width[r] == 0 ) );
		printf("Row: %d - Start: %d.\n", r, row_start[r] );
	}
}

void vfont_bindGlyph( const char c ) {
	//vglBindTexture( g_glyph[(unsigned int)c] );
	vglBindTexture( g_atlas );
}

static const float stride = 16.f;

void font_renderGlyph( float x, float y, const char c ) {
	vector from = Vector( x, y, g_glyphData[(uchar)c].coord.x, g_glyphData[(uchar)c].coord.y );
	vector to = Vector( x + stride, y + stride, g_glyphData[(uchar)c].coord.z, g_glyphData[(uchar)c].coord.w );
	vfont_bindGlyph( c );
	debugdraw_drawRect2D( &from, &to );
}

void font_renderString( float x, float y, String string ) {
	int l = strlen( string );
	assert( l > 0 );
	for ( int i = 0; i < l; i++ ) {
		font_renderGlyph( x, y, string[i] );
		x += stride;
	}
}
