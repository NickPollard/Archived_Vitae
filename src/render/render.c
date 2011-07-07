// render.c

#include "common.h"
#include "render.h"
//-----------------------
#include "camera.h"
#include "font.h"
#include "light.h"
#include "model.h"
#include "scene.h"
#include "render/debugdraw.h"
#include "render/modelinstance.h"
#include "render/texture.h"
#include "system/file.h"
// temp
#include "engine.h"

// GLFW Libraries
#include <GL/glfw.h>

// *** Shader Pipeline


gl_resources resources;

GLuint gl_bufferCreate( GLenum target, const void* data, GLsizei size ) {
	GLuint buffer; // The OpenGL object handle we generate
	glGenBuffers( 1, &buffer );
	glBindBuffer( target, buffer );
	// Usage hint can be: GL_[VARYING]_[USE]
	// Varying: STATIC / DYNAMIC / STREAM
	// Use: DRAW / READ / COPY
	glBufferData( target, size, data, /*Usage hint*/ GL_STATIC_DRAW );
	return buffer;
}

GLfloat vertex_buffer_data[256];
GLushort element_buffer_data[256];

void render_initResources() {
	vertex_buffer_data[0] = 1.0f;
	vertex_buffer_data[1] = 0.0f;
	vertex_buffer_data[2] = 0.0f;
	vertex_buffer_data[3] = 1.0f;

	vertex_buffer_data[4] = 0.0f;
	vertex_buffer_data[5] = 1.0f;
	vertex_buffer_data[6] = 0.0f;
	vertex_buffer_data[7] = 1.0f;

	vertex_buffer_data[8] = -1.0f;
	vertex_buffer_data[9] = 0.0f;
	vertex_buffer_data[10] = 0.0f;
	vertex_buffer_data[11] = 1.0f;

	vertex_buffer_data[12] = 0.0f;
	vertex_buffer_data[13] = -1.0f;
	vertex_buffer_data[14] = 0.0f;
	vertex_buffer_data[15] = 1.0f;

	element_buffer_data[0] = 0;
	element_buffer_data[1] = 1;
	element_buffer_data[2] = 3;
	element_buffer_data[3] = 2;

	// OpenGL allocates space for our buffers, copies our data into them
	resources.vertex_buffer = gl_bufferCreate( GL_ARRAY_BUFFER, vertex_buffer_data, sizeof( vertex_buffer_data ) );
	resources.element_buffer = gl_bufferCreate( GL_ELEMENT_ARRAY_BUFFER, element_buffer_data, sizeof( element_buffer_data ) );
}

void render_setBuffers( float* vertex_buffer, int vertex_buffer_size, int* element_buffer, int element_buffer_size ) {
	resources.vertex_buffer = gl_bufferCreate( GL_ARRAY_BUFFER, vertex_buffer, vertex_buffer_size );
	resources.element_buffer = gl_bufferCreate( GL_ELEMENT_ARRAY_BUFFER, element_buffer, element_buffer_size );
}

typedef void (*func_getIV)( GLuint, GLenum, GLint* );
typedef void (*func_getInfoLog)( GLuint, GLint, GLint*, GLchar* );

void gl_dumpInfoLog( GLuint object, func_getIV getIV, func_getInfoLog getInfoLog ) {
	GLint length;
	char* log;
	getIV( object, GL_INFO_LOG_LENGTH, &length );
	log = mem_alloc( sizeof( char ) * length );
	getInfoLog( object, length, NULL, log );
	printf( "--- Begin Info Log ---\n" );
	printf( "%s", log );
	printf( "--- End Info Log ---\n" );
	mem_free( log );
}

// Based on code from Joe's Blog: http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html
GLuint render_compileShader( GLenum type, const char* path ) {
	GLint length;
	const char* source = vfile_contents( path, &length );
	GLuint shader;
	GLint shader_ok;

	if ( !source ) {
		printf( "Error: Cannot create Shader. File %s not found.\n", path );
		assert( 0 );
	}

	shader = glCreateShader( type );
	glShaderSource( shader, 1, (const GLchar**)&source, &length );
	mem_free( (void*)source );
	glCompileShader( shader );

	glGetShaderiv( shader, GL_COMPILE_STATUS, &shader_ok );
	if ( !shader_ok) {
		printf( "Error: Failed to compile Shader from File %s.\n", path );
		gl_dumpInfoLog( shader, glGetShaderiv,  glGetShaderInfoLog);
		assert( 0 );
	}

	return shader;
}

GLuint render_linkShaderProgram( GLuint vertex_shader, GLuint fragment_shader ) {
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

void render_buildShaders() {
	resources.vertex_shader = render_compileShader( GL_VERTEX_SHADER, "dat/shaders/phong_vertex.glsl" );
	resources.fragment_shader = render_compileShader( GL_FRAGMENT_SHADER, "dat/shaders/phong_fragment.glsl" );

	resources.program = render_linkShaderProgram( resources.vertex_shader, resources.fragment_shader );

	// Uniforms
	resources.uniforms.projection = glGetUniformLocation( resources.program, "projection" );
	resources.uniforms.modelview = glGetUniformLocation( resources.program, "modelview" );

	// Attributes
	resources.attributes.position = glGetAttribLocation( resources.program, "position" );
}
// Private Function declarations

// *** Fixed Function Pipeline

void render_set3D( int w, int h ) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glDepthMask( GL_TRUE );
}

void render_set2D() {
	// *** Set an orthographic projection
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0,		// left
			640.0,	// right
		   	480.0,	// bottom
		   	0,		// top
		   	0,		// near
		   	1 );	// far

	glDisable( GL_DEPTH_TEST );
}

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s) {
	for (int i = 0; i < s->model_count; i++) {
		modelInstance_draw( scene_model( s, i ));
	}
}

void render_lighting( scene* s ) {
	// Ambient Light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, s->ambient);

	// Point Lights	
	// according to www.opengl.org/sdk/dorcs/man/xhtml/glLight.xml this should work
	for ( int i = 0; i < s->light_count; i++)
		light_render( GL_LIGHT0 + i /* according to the oGL spec, this should work */, s->lights[i] );
}

void render_applyCamera(camera* cam) {
	// Negate as we're doing the inverse of camera
	matrix cam_inverse;
	matrix_inverse( cam_inverse, cam->trans->world );
	glMultMatrixf( (float*)cam_inverse );
}

// Clear information from last draw
void render_clear() {
	glClearColor(0.f, 0.f, 0.0, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Initialise the 3D rendering
void render_init() {
	if (!glfwInit())
		printf("ERROR - failed to init glfw.\n");

	glfwOpenWindow(640, 480, 8, 8, 8, 8, 8, 0, GLFW_WINDOW);
	glfwSetWindowTitle("Vitae");
	glfwSetWindowSizeCallback(handleResize);

	printf("RENDERING: Initialising OpenGL rendering settings.\n");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
//	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);

	texture_init();

	render_initResources();
	render_buildShaders();
}

// Terminate the 3D rendering
void render_terminate() {
	glfwTerminate();
}

void render_shader( scene* s );

// Render the current scene
// This is where the business happens
void render( scene* s, int w, int h ) {
/*
	render_set3D( w, h );

	// Switch to the drawing perspective and initialise to the identity matrix
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
	render_applyCamera( s->cam );
	glColor4f(1.f, 1.f, 1.f, 1.f);

	render_lighting( s );
	render_scene( s );
	*/
	render_shader( s );
}

void render_perspectiveMatrix( matrix m, float fov, float aspect, float near, float far ) {
	matrix_setIdentity( m );
	m[0][0] = 1 / tan( fov );
	m[1][1] = aspect / tan( fov );
	m[2][2] = ( far + near )/( far - near );
	m[2][3] = 1.f;
	m[3][2] = -2.f * far * near / (far - near );
	m[3][3] = 0.f;
}

// Shader version
void render_shader( scene* s ) {
	// Load our shader
	glUseProgram( resources.program );

	matrix projection;
	matrix modelview;

	matrix_setIdentity( projection );
	matrix_setIdentity( modelview );

	float fov = 120.f;
	float aspect = 4.f/3.f;
	float z_near = 1.f;
	float z_far = 200.f;
	matrix perspective;
	render_perspectiveMatrix( perspective, fov, aspect, z_near, z_far );

	matrix cam_inverse;
	matrix_inverse( cam_inverse, s->cam->trans->world );
	matrix_mul( projection, perspective, cam_inverse );

	// Set up uniforms
	glUniformMatrix4fv( resources.uniforms.projection, 1, /*transpose*/false, (GLfloat*)projection );
	glUniformMatrix4fv( resources.uniforms.modelview, 1, /*transpose*/false, (GLfloat*)modelview );

	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
	glVertexAttribPointer( resources.attributes.position, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof(GLfloat)*4, (void*)0 );
	glEnableVertexAttribArray( resources.attributes.position );

//	int count = 4;
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
//	glDrawElements( GL_TRIANGLE_STRIP, count, GL_UNSIGNED_SHORT, (void*)0 );

	glDisableVertexAttribArray( resources.attributes.position );

	render_scene( s );
}
