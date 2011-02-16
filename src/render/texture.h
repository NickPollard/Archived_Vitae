// texture.h
#ifndef __TEXTURE_H__
#define __TEXTURE_H__

// GLFW Libraries
#include <GL/glfw.h>

// Globals
extern GLuint g_texture_default;

void texture_init();

GLuint texture_loadTGA(const char* filename);

#endif // __TEXTURE_H__
