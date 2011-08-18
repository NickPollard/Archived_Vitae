// shader.c

#include "common.h"
#include "render/shader.h"
//---------------------
#include "mem/allocator.h"
#include "render/render.h"
#include "system/file.h"
#include "system/string.h"
/*
// Activate the shader for use in rendering
void shader_activate( shader* s ) {
	// Bind the shader program for use
	glUseProgram( s->program );

	// Set up shader constants ( GLSL Uniform variables )
	for ( each shader_constant in shader ) {
		GLuint location = glGetUniformLocation( program, shader_constant );
		render_setUniform( location, render_getConstant( shader_constant ));
	}
}

// Load a shader from GLSL files
shader* shader_load( const char* vertex_name, const char* fragment_name ) {
	shader* s = mem_alloc( sizeof( shader ));
	memset( s, 0, sizeof( shader ));
	s->program = render_buildShader( vertex_name, fragment_name );

	shader_buildDictionary( vertex_name );
	shader_buildDictionary( fragment_name );
	return s;
}

*/
typedef struct shaderConstantBinding_s {
	void*		value;
	GLint		program_location;
} shaderConstantBinding;

// Register the shader constant of the given name, returning it's address, or if it
// already exists then use that
void* shader_registerConstant( const char* name ) {
//	hash_findOrAdd( shader_constants, name );
	return NULL;
	/*
	// TODO should be a hashmap eventually
	for ( int i = 0; i < shaderconstant_count; i++ ) {
		if ( shader_constant.name = name )
			return shader_constant.value
	}
	return NULL;
	*/
}

// Find the program location for a named Uniform variable in the given program
GLint shader_getUniformLocation( GLuint program, const char* name ) {
	GLint location = glGetUniformLocation( program, name );
	if ( location == -1 ) {
		printf( "Error finding Uniform Location for shader uniform variable \"%s\".\n", name );
	}
	assert( location != -1 );
	return location;
}

// Create a binding for the given variable within the given program
void shader_createBinding( GLuint shader_program, const char* variable_name ) {
	// For each one create a binding
	shaderConstantBinding binding;
	binding.program_location = shader_getUniformLocation( shader_program, variable_name );
	binding.value = shader_registerConstant( variable_name );
	printf( "SHADER: Created Shader binding for \"%s\" at location 0x%x\n", variable_name, binding.program_location );
}

// Find a list of uniform variable names in a shader source file
void shader_findUniforms( GLuint shader_program, const char* src ) {
	inputStream* stream = inputStream_create( src );
	char* token;
	while ( !inputStream_endOfFile( stream )) {
		token = inputStream_nextToken( stream );
		if ( string_equal( token, "uniform" ) && !inputStream_endOfFile( stream )) {
			// Advance two tokens (the next is the type declaration, the second is the variable name)
			mem_free( token );
			inputStream_skipToken( stream ); // Skip the variable type declaration
			token = inputStream_nextToken( stream );
			// Now we have the name
			const char* name = string_trim( token );
			shader_createBinding( shader_program, name );
			mem_free( (void*)name );
		}
		mem_free( token );
	}
}

// Compile a GLSL shader object from the given source code
// Based on code from Joe's Blog: http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html
GLuint shader_compile( GLenum type, const char* path, const char* source ) {
	GLint length;
	GLuint shader;
	GLint shader_ok;

	if ( !source ) {
		printf( "Error: Cannot create Shader. File %s not found.\n", path );
		assert( 0 );
	}

	shader = glCreateShader( type );
	glShaderSource( shader, 1, (const GLchar**)&source, &length );
	glCompileShader( shader );

	glGetShaderiv( shader, GL_COMPILE_STATUS, &shader_ok );
	if ( !shader_ok) {
		printf( "Error: Failed to compile Shader from File %s.\n", path );
		gl_dumpInfoLog( shader, glGetShaderiv,  glGetShaderInfoLog);
		assert( 0 );
	}

	return shader;
}

// Link two given shader objects into a full shader program
GLuint shader_link( GLuint vertex_shader, GLuint fragment_shader ) {
	GLint program_ok;

	GLuint program = glCreateProgram();
	glAttachShader( program, vertex_shader );
	glAttachShader( program, fragment_shader );
	glLinkProgram( program );

	glGetProgramiv( program, GL_LINK_STATUS, &program_ok );
	if ( !program_ok ) {
		printf( "Failed to link shader program.\n" );
		gl_dumpInfoLog( program, glGetProgramiv,  glGetProgramInfoLog);
		glDeleteProgram( program );
		assert( 0 );
	}

	return program;
}

// Build a GLSL shader program from given vertex and fragment shader source pathnames
GLuint	shader_build( const char* vertex_path, const char* fragment_path ) {
	int length = 0;
	const char* vertex_file = vfile_contents( vertex_path, &length );
	const char* fragment_file = vfile_contents( fragment_path, &length );
	GLuint vertex_shader = shader_compile( GL_VERTEX_SHADER, vertex_path, vertex_file );
	GLuint fragment_shader = shader_compile( GL_FRAGMENT_SHADER, fragment_path, fragment_file );
	GLuint program = shader_link( vertex_shader, fragment_shader );
	shader_findUniforms( program, vertex_file );
	shader_findUniforms( program, fragment_file );
	mem_free( (void*)vertex_file );			// Cast away const to free, we allocated this ourselves
	mem_free( (void*)fragment_file	);		// Cast away const to free, "
	return program;
}
/*
// Build a dictionary of shader constant lookups
void shader_buildDictionary( const char* src, GLuint shader_program ) {
#define MAX_SHADER_CONSTANT_NAMES 64
	int shader_constant_count;
	const char* names[MAX_SHADER_CONSTANT_NAMES]
	// Find a list of uniform variable names



	const char* variable_name;

	// For each one create a binding
	shaderConstantBinding binding;
	binding.program_location = glGetUniformLocation( shader_program, variable_name );
	binding.value = shaderconstant_lookup( variable_name );

}

void shader_bindConstants( shader* s ) {
	for (every shader_constant) {
		value = shaderconstant_lookup( shader_constant.uniform_name );
		setUniform( shader_constant.program_location, value );
	}
}
*/
