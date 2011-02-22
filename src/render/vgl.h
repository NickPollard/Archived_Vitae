// vgl.h
#pragma once

//
// Vitae Graphics Library - a wrapper around a lower level graphics library (OpenGL)
// Used by the Vitae game engine to avoid binding too tightly to a platform
//

// A vglTexture Handle
// in this case maps to a GLuint
typedef u32 vglTexture;

// *** Defines

#define VGL_LUMINANCE GL_LUMINANCE 
#define VGL_SAMPLE_LINEAR GL_LINEAR 

vglTexture vgl_buildTexture( const ubyte* img, int w, int h, int type, int format );
