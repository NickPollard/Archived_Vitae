// shader.h
#pragma once

// GLFW Libraries
#include <GL/glfw.h>

typedef struct shader_s {
	GLuint program;				// The Linked OpenGL shader program, containing vertex and fragment shaders;
//	shaderDictionary	dict;	// Dictionary of shader constant lookups
} shader;

// Compile a GLSL shader object from the given source code
GLuint shader_compile( GLenum type, const char* path, const char* source );

// Build a GLSL shader program from given vertex and fragment shader source pathnames
GLuint	shader_build( const char* vertex_path, const char* fragment_path );

// Find the program location for a named Uniform variable in the given program
GLint shader_getUniformLocation( GLuint program, const char* name );
