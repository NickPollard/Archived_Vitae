// shader.h
#pragma once

// GLFW Libraries
#include <GL/glfw.h>

typedef struct shader_s {
	GLuint program;				// The Linked OpenGL shader program, containing vertex and fragment shaders;
//	shaderDictionary	dict;	// Dictionary of shader constant lookups
} shader;

// temp TODO: remove
void shader_findUniforms( const char* src );
