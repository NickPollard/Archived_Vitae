// texture.c

#include "common.h"
#include "render/texture.h"
//---------------------
#include "external/util.h"

// GLFW Libraries
#include <GL/glfw.h>

// Globals
GLuint g_texture_default = 0;

void texture_init() {
	g_texture_default = texture_loadTGA( "assets/img/test64rgb.tga" );
	printf( "Loaded default texture as OpenGL texture name: %d\n", g_texture_default );
}

GLuint texture_loadTGA( const char* filename ) {
	GLuint tex;
	int w, h;
	void* img = read_tga( filename, &w, &h );

	if ( !img )
		return 0;	// Failed to load the texture

	// Generate a texture name and bind to that
	glGenTextures( 1, &tex );
	glBindTexture( GL_TEXTURE_2D, tex );

	// Set up sampling parameters, use defaults for now
	// Bilinear interpolation, clamped
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );

	glTexImage2D( GL_TEXTURE_2D,
		   			0,			// No Mipmaps for now
					GL_RGBA8,	// 3-channel, 8-bits per channel (32-bit stride)
					(GLsizei)w, (GLsizei)h,
					0,			// Border, unused
					GL_BGRA,		// TGA uses BGR order internally
					GL_UNSIGNED_BYTE,	// 8-bits per channel
					img );

	free(img);	// OpenGL copies the data, so we can free this here

	return tex;
}
