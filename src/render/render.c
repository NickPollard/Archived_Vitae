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
#include "render/shader.h"
#include "render/texture.h"
#include "system/file.h"
#include "system/hash.h"
// temp
#include "engine.h"

// GLFW Libraries
#include <GL/glfw.h>

#define MAX_VERTEX_ARRAY_COUNT 1024

// *** Shader Pipeline

matrix modelview, camera_inverse;
matrix perspective;

gl_resources resources;

GLuint gl_bufferCreate( GLenum target, const void* data, GLsizei size ) {
	GLuint buffer; // The OpenGL object handle we generate
	glGenBuffers( 1, &buffer );
	glBindBuffer( target, buffer );
	// Usage hint can be: GL_[VARYING]_[USE]
	// Varying: STATIC / DYNAMIC / STREAM
	// Use: DRAW / READ / COPY
	glBufferData( target, size, data, /*Usage hint*/ GL_DYNAMIC_DRAW ); // OpenGL ES only supports dynamic/static draw
	return buffer;
}

void render_setBuffers( float* vertex_buffer, int vertex_buffer_size, int* element_buffer, int element_buffer_size ) {
	resources.vertex_buffer = gl_bufferCreate( GL_ARRAY_BUFFER, vertex_buffer, vertex_buffer_size );
	resources.element_buffer = gl_bufferCreate( GL_ELEMENT_ARRAY_BUFFER, element_buffer, element_buffer_size );
}

void render_buildShaders() {
	resources.shader_default = shader_load( "dat/shaders/phong.v.glsl", "dat/shaders/phong.f.glsl" );
	resources.shader_particle = shader_load( "dat/shaders/textured_phong.v.glsl", "dat/shaders/textured_phong.f.glsl" );

#define GET_UNIFORM_LOCATION( var ) \
	resources.uniforms.var = shader_findConstant( mhash( #var )); \
	assert( resources.uniforms.var != NULL );
	SHADER_UNIFORMS( GET_UNIFORM_LOCATION )

#define GET_UNIFORM_LOCATION_PARTICLE( var ) \
	resources.uniforms.var = shader_findConstant( mhash( #var ));
	SHADER_UNIFORMS( GET_UNIFORM_LOCATION_PARTICLE )
/*
	// Attributes
	resources.attributes.position = shader_getAttributeLocation( resources.shader_particle->program, "position" );
	resources.attributes.normal = shader_getAttributeLocation( resources.shader_particle->program, "normal" );
	resources.attributes.uv = shader_getAttributeLocation( resources.shader_particle->program, "uv" );
	*/
}

// Private Function declarations

void render_set3D( int w, int h ) {
	glViewport(0, 0, w, h);
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glDepthMask( GL_TRUE );
}

void render_set2D() {
#ifndef OPENGL_ES
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
#endif
}

#ifdef ANDROID
void render_swapBuffers( egl_renderer* egl ) {
    eglSwapBuffers( egl->display, egl->surface );
#else
void render_swapBuffers() {
	glfwSwapBuffers(); // Send the 3d scene to the screen (flips display buffers)
#endif // ANDROID
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

	// Point Lights	
	light_renderLights( s->light_count, s->lights );
}

// Clear information from last draw
void render_clear() {
	glClearColor( 0.f, 0.f, 0.0, 0.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void render_initWindow() {
#ifndef ANDROID
	if (!glfwInit())
		printf("ERROR - failed to init glfw.\n");

	glfwOpenWindow(640, 480, 8, 8, 8, 8, 8, 0, GLFW_WINDOW);
	glfwSetWindowTitle("Vitae");
	glfwSetWindowSizeCallback(handleResize);
#endif
}

// Initialise the 3D rendering
void render_init() {
	render_initWindow();

	printf("RENDERING: Initialising OpenGL rendering settings.\n");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );	// Standard Alpha Blending

	// Backface Culling
	glFrontFace( GL_CW );
	glEnable( GL_CULL_FACE );

	texture_init();

	shader_init();

	render_buildShaders();
	
	// Allocate space for buffers
	const GLsizei vertex_buffer_size = sizeof( vector ) * MAX_VERTEX_ARRAY_COUNT;
	const GLsizei element_buffer_size = sizeof( GLushort ) * MAX_VERTEX_ARRAY_COUNT;
	resources.vertex_buffer = gl_bufferCreate( GL_ARRAY_BUFFER, NULL, vertex_buffer_size );
	resources.element_buffer = gl_bufferCreate( GL_ELEMENT_ARRAY_BUFFER, NULL, element_buffer_size );
}

// Terminate the 3D rendering
void render_terminate() {
#ifndef ANDROID
	glfwTerminate();
#endif
}

void render_shader( scene* s );

// Render the current scene
// This is where the business happens
void render( scene* s, int w, int h ) {
	render_shader( s );
}

void render_perspectiveMatrix( matrix m, float fov, float aspect, float near, float far ) {
	matrix_setIdentity( m );
	m[0][0] = 1 / tan( fov );
	m[1][1] = aspect / tan( fov );
	m[2][2] = ( far + near )/( far - near );
	m[2][3] = 1.f;
	m[3][2] = (-2.f * far * near) / (far - near );
	m[3][3] = 0.f;
}

void render_validateMatrix( matrix m ) {
	for ( int i = 0; i < 3; i++ ) {
		assert( isNormalized( matrix_getCol( m, i ) ));
	}
	assert( f_eq( m[0][3], 0.f ));
	assert( f_eq( m[1][3], 0.f ));
	assert( f_eq( m[2][3], 0.f ));
	assert( f_eq( m[3][3], 1.f ));
}

void render_resetModelView( ) {
	matrix_cpy( modelview, camera_inverse );
}

void render_setUniform_matrix( GLuint uniform, matrix m ) {
	glUniformMatrix4fv( uniform, 1, /*transpose*/false, (GLfloat*)m );
}

// Takes a uniform and an OpenGL texture name (GLuint)
// It binds the given texture to an available texture unit
// and sets the uniform to that
void render_setUniform_texture( GLuint uniform, GLuint texture ) {
	// Activate a texture unit
	glActiveTexture( GL_TEXTURE0 );
	// Bind the texture to that texture unit
	glBindTexture( GL_TEXTURE_2D, texture );
	glUniform1i( uniform, 0 );

}


// Shader version
void render_shader( scene* s ) {
	// Load our shader
	shader_activate( resources.shader_default );
	matrix_setIdentity( modelview );

	const float fov = 0.8f; // In radians
	const float aspect = 4.f/3.f;
	const float z_near = 1.f;
	const float z_far = 500.f;
	render_perspectiveMatrix( perspective, fov, aspect, z_near, z_far );

	camera* cam = s->cam;
	render_validateMatrix( cam->trans->world );
	matrix_inverse( camera_inverse, cam->trans->world );
	render_validateMatrix( modelview );
	render_resetModelView();

	// Set up uniforms
	render_setUniform_matrix( *resources.uniforms.projection, perspective );
	render_setUniform_matrix( *resources.uniforms.modelview, modelview );
	render_setUniform_matrix( *resources.uniforms.worldspace, modelview );

	// Textures
//	render_setUniform_texture( *resources.uniforms.tex, g_texture_default );

	render_lighting( s );

	render_scene( s );
}
