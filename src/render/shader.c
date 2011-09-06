// shader.c

#include "common.h"
#include "render/shader.h"
//---------------------
#include "mem/allocator.h"
#include "render/render.h"
#include "system/file.h"
#include "system/string.h"
#include "system/hash.h"

#define kMaxShaderConstants 128
map* shader_constants = NULL;

// Find the program location for a named Uniform variable in the given program
GLint shader_getUniformLocation( GLuint program, const char* name ) {
	GLint location = glGetUniformLocation( program, name );
/*
   if ( location == -1 ) {
		printf( "Error finding Uniform Location for shader uniform variable \"%s\".\n", name );
	}
	assert( location != -1 );
*/
	return location;
}

// Find the program location for a named Attribute variable in the given program
GLint shader_getAttributeLocation( GLuint program, const char* name ) {
	GLint location = glGetAttribLocation( program, name );
	printf( "SHADER: Attribute \"%s\" location: 0x%x\n", name, location );
	return location;
}

// Create a binding for the given variable within the given program
shaderConstantBinding shader_createBinding( GLuint shader_program, const char* variable_type, const char* variable_name ) {
	// For each one create a binding
	shaderConstantBinding binding;
	binding.program_location = shader_getUniformLocation( shader_program, variable_name );
	if ( binding.program_location == -1 )
		binding.program_location = shader_getAttributeLocation( shader_program, variable_name );
	binding.value = map_findOrAdd( shader_constants, mhash( variable_name ));

	// TODO: Make this into an easy lookup table
	binding.type = uniform_unknown;
	if ( string_equal( "vec4", variable_type ))
		binding.type = uniform_vector;
	if ( string_equal( "mat4", variable_type ))
		binding.type = uniform_matrix;
	if ( string_equal( "sampler2D", variable_type ))
		binding.type = uniform_tex2D;

	printf( "SHADER: Created Shader binding 0x%x for \"%s\" at location 0x%x, type: %s\n", (unsigned int)binding.value, variable_name, binding.program_location, variable_type );
	return binding;
}

void shaderDictionary_addBinding( shaderDictionary* d, shaderConstantBinding b ) {
	assert( d->count < kMaxShaderConstantBindings );
	d->bindings[d->count++] = b;
}

// Find a list of uniform variable names in a shader source file
void shader_buildDictionary( shaderDictionary* dict, GLuint shader_program, const char* src ) {
	printf( "SHADER: Building Shader Dictionary.\n" );
	// Find a list of uniform variable names
	inputStream* stream = inputStream_create( src );
	char* token;
	while ( !inputStream_endOfFile( stream )) {
		token = inputStream_nextToken( stream );
		if (( string_equal( token, "uniform" ) || string_equal( token, "attribute" )) && !inputStream_endOfFile( stream )) {
			// Advance two tokens (the next is the type declaration, the second is the variable name)
			mem_free( token );
			token = inputStream_nextToken( stream );
			const char* type = string_trim( token );
			mem_free( token );
			token = inputStream_nextToken( stream );
			const char* name = string_trim( token );
			// If it's an array remove the array specification
			char* c = (char*)name;
			int length = strlen(name );
			while ( c < name + length ) {
				if ( *c == '[' )
					*c = '\0';
				c++;
			}

			shaderDictionary_addBinding( dict, shader_createBinding( shader_program, type, name ));

			mem_free( (void*)name );
			mem_free( (void*)type );
		}
		mem_free( token );
	}
}

// Compile a GLSL shader object from the given source code
// Based on code from Joe's Blog: http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html
GLuint shader_compile( GLenum type, const char* path, const char* source ) {
	GLint length = strlen( source );
	GLuint glShader;
	GLint shader_ok;

	if ( !source ) {
		printf( "Error: Cannot create Shader. File %s not found.\n", path );
		assert( 0 );
	}

	glShader = glCreateShader( type );
	glShaderSource( glShader, 1, (const GLchar**)&source, &length );
	glCompileShader( glShader );

	glGetShaderiv( glShader, GL_COMPILE_STATUS, &shader_ok );
	if ( !shader_ok) {
		printf( "Error: Failed to compile Shader from File %s.\n", path );
		gl_dumpInfoLog( glShader, glGetShaderiv,  glGetShaderInfoLog);
		assert( 0 );
	}

	return glShader;
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
GLuint	shader_build( const char* vertex_path, const char* fragment_path, const char* vertex_file, const char* fragment_file ) {
	GLuint vertex_shader = shader_compile( GL_VERTEX_SHADER, vertex_path, vertex_file );
	GLuint fragment_shader = shader_compile( GL_FRAGMENT_SHADER, fragment_path, fragment_file );
	GLuint program = shader_link( vertex_shader, fragment_shader );
	return program;
}

void shader_init() {
	shader_constants = map_create( kMaxShaderConstants, sizeof( GLint ));
}

void shader_bindConstant( shaderConstantBinding binding ) {
	*binding.value = binding.program_location;
}

GLint* shader_findConstant( int key ) {
	return map_find( shader_constants, key );
}

void shader_bindConstants( shader* s ) {
	assert( s->dict.count < kMaxShaderConstantBindings );
	for ( int i = 0; i < s->dict.count; i++ ) {
		shader_bindConstant( s->dict.bindings[i] );
	}
}

// Activate the shader for use in rendering
void shader_activate( shader* s ) {
	// Bind the shader program for use
	glUseProgram( s->program );

	// Set up shader constants ( GLSL Uniform variables )
	shader_bindConstants( s );
}

// Load a shader from GLSL files
shader* shader_load( const char* vertex_name, const char* fragment_name ) {
	printf( "SHADER: Loading Shader (Vertex: \"%s\", Fragment: \"%s\")\n", vertex_name, fragment_name );
	shader* s = mem_alloc( sizeof( shader ));
	memset( s, 0, sizeof( shader ));
	s->dict.count = 0;

	// Load source code
	int length = 0;
	const char* vertex_file = vfile_contents( vertex_name, &length );
	const char* fragment_file = vfile_contents( fragment_name, &length );

	// Build the shader program
	s->program = shader_build( vertex_name, fragment_name, vertex_file, fragment_file );

	// Build our dictionaries
	shader_buildDictionary( &s->dict, s->program, vertex_file );
	shader_buildDictionary( &s->dict, s->program, fragment_file );

	// Clear up memory
	mem_free( (void*)vertex_file );			// Cast away const to free, we allocated this ourselves
	mem_free( (void*)fragment_file	);		// Cast away const to free, "

	return s;
}
