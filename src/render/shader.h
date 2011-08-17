// shader.h
#pragma once

// GLFW Libraries
#include <GL/glfw.h>

typedef struct shader_s {
	GLuint program;				// The Linked OpenGL shader program, containing vertex and fragment shaders;
//	shaderDictionary	dict;	// Dictionary of shader constant lookups
} shader;

// temp TODO: remove
void shader_findUniforms( GLuint program, const char* src );
GLuint shader_compile( GLenum type, const char* path, const char* source );
GLuint	shader_build( const char* vertex_path, const char* fragment_path );
GLint shader_getUniformLocation( GLuint program, const char* name );
