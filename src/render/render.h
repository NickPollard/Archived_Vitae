// render.h

#include "scene.h"

typedef struct gl_resources_s {
	GLuint vertex_buffer, element_buffer;
	GLuint texture;

	struct {
		GLuint projection;
		GLuint modelview;
		GLuint light_position;
		GLuint light_diffuse;
	} uniforms;

	struct {
		GLint position;
		GLint normal;
	} attributes;

	// Shader objects
	GLuint vertex_shader, fragment_shader, program;
} gl_resources;

extern gl_resources resources;
extern matrix modelview;

void render_setBuffers( float* vertex_buffer, int vertex_buffer_size, int* element_buffer, int element_buffer_size );

/*
 *
 *  Static Functions
 *
 */

void render_set2D();

void render_set3D( int w, int h );

void render_clear();

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s);

void render_lighting(scene* s);

void render_applyCamera(camera* cam);

// Initialise the 3D rendering
void render_init();

// Terminate the 3D rendering
void render_terminate();

// Render the current scene
// This is where the business happens
void render( scene* s , int w, int h );

void render_resetModelView( );
