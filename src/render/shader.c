// shader.c

#include "common.h"
#include "render/shader.h"
//---------------------
#include "mem/allocator.h"
#include "system/file.h"
#include "system/string.h"
/*
// Activate the shader for use in rendering
void shader_activate( shader* s ) {
	// Bind the shader program for use
	glUseProgram( s->program );

	// Set up shader constants ( GLSL Uniform variables )
	for ( each shader_constant in shader ) {
		GLuint location = getUniformLocation( program, shader_constant );
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

typedef struct shaderConstantBinding_s {
	void*		value;
	GLint		program_location;
} shaderConstantBinding;
*/
// Find a list of uniform variable names in a shader source file
void shader_findUniforms( const char* src ) {
	inputStream* stream = inputStream_create( src );
	char* token;
	while ( !inputStream_endOfFile( stream )) {
		token = inputStream_nextToken( stream );
		if ( string_equal( token, "uniform" ) && !inputStream_endOfFile( stream )) {
			mem_free( token );
			token = inputStream_nextToken( stream );
			printf( "Found Uniform variable named \"%s\".\n", token );
		}
		mem_free( token );
	}
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
	binding.program_location = getUniformLocation( shader_program, variable_name );
	binding.value = shaderconstant_lookup( variable_name );

}

// Return the currently bound value of the given shader constant
void* shaderconstant_lookup( const char* name ) {
	// TODO should be a hashmap eventually
	for ( int i = 0; i < shaderconstant_count; i++ ) {
		if ( shader_constant.name = name )
			return shader_constant.value
	}
	return NULL;
}

void shader_bindConstants( shader* s ) {
	for (every shader_constant) {
		value = shaderconstant_lookup( shader_constant.uniform_name );
		setUniform( shader_constant.program_location, value );
	}
}
*/
