// font.c

#include "common.h"
#include "font.h"
//-----------------------
#include "maths.h"
#include "external/util.h"
#include "render/debugdraw.h"
// GLFW Libraries
#include <GL/glfw.h>

// *** The stb TrueType library
#define STB_TRUETYPE_IMPLEMENTATION
#include  "3rdparty/stbTrueType/stb_truetype.h"

unsigned char ttf_buffer[1<<25];
   
GLuint g_glyph[256];

void test_font_renderChar( char* c ) {
	
}

#define FONT_PATH "/usr/share/fonts/truetype/ttf-droid/DroidSans.ttf"

// Test init function, based on the main func in the complete program implementation of stb TrueType ((C) Sean Barrett 2009)
void font_init() {
   stbtt_fontinfo font;
   unsigned char *bitmap;
//   int w,h,i,j,c = (argc > 1 ? atoi(argv[1]) : 'a'), s = (argc > 2 ? atoi(argv[2]) : 20);
   int w,h,c,s;
   c = 'a';
   s = 20;

   // *** load the ttf file
   int ret = fread(ttf_buffer, 1, 1<<25, fopen(FONT_PATH, "rb"));
   (void)ret;

   // *** init the tt system and extract font data
   stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

   glGenTextures( 256, g_glyph );

   for (unsigned char ch = 0; ch < 255; ch++) {
	   // *** extract a bitmap
	   bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), ch, &w, &h, 0,0);
	   unsigned char img[16][16];
	   memset( img, 0, 16*16 );
	   for ( int i = 0 ; (i < h) && (i < 16); i++ ) {
		   memcpy( img[i], bitmap + (w * i), w );
	   }
	   printf( "Glyph: '%c': w %d h %d.\n", ch, w, h );
	   glBindTexture( GL_TEXTURE_2D, g_glyph[ch] );

	   // Set up sampling parameters, use defaults for now
	   // Bilinear interpolation, clamped
	   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
	   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );

	   glTexImage2D( GL_TEXTURE_2D,
			   0,			// No Mipmaps for now
			   GL_LUMINANCE,	// 1-channel, 8-bits per channel (8-bit stride)
			   (GLsizei)16, (GLsizei)16,
			   0,			// Border, unused
			   GL_LUMINANCE,		// TGA uses BGR order internally
			   GL_UNSIGNED_BYTE,	// 8-bits per channel
			   img );

	   // *** display the bitmap
	   /*
		  int i, j;
		  for (j=0; j < h; ++j) {
		  for (i=0; i < w; ++i)
		  putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
		  putchar('\n');
		  }
		  */
	   free( bitmap );
   }
}

void font_bindGlyph( const char c ) {
	glBindTexture( GL_TEXTURE_2D, g_glyph[(unsigned int)c] );
}

static const float stride = 16.f;

void font_renderGlyph( float x, float y, const char c ) {
	vector from = Vector( x, y, 0.f, 0.f );
	vector to = Vector( x + stride, y + stride, 0.f, 0.f );
	font_bindGlyph( c );
	debugdraw_drawRect2D( &from, &to );
}

void font_renderString( float x, float y, const char* string ) {
	int l = strlen( string );
	assert( l > 0 );
	for ( int i = 0; i < l; i++ ) {
		font_renderGlyph( x, y, string[i] );
		x += stride;
	}
}
