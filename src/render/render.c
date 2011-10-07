// render.c

#include "common.h"
#include "render.h"
//-----------------------
#include "camera.h"
#include "font.h"
#include "light.h"
#include "model.h"
#include "scene.h"
#include "skybox.h"
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

bool	render_initialised = false;
vmutex	gl_mutex = kMutexInitialiser;

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

/*
void render_setBuffers( float* vertex_buffer, int vertex_buffer_size, int* element_buffer, int element_buffer_size ) {
	resources.vertex_buffer = gl_bufferCreate( GL_ARRAY_BUFFER, vertex_buffer, vertex_buffer_size );
	resources.element_buffer = gl_bufferCreate( GL_ELEMENT_ARRAY_BUFFER, element_buffer, element_buffer_size );
}
*/

void render_buildShaders() {
	// Load Shaders								Vertex								Fragment
	resources.shader_default	= shader_load( "dat/shaders/phong.v.glsl",			"dat/shaders/phong.f.glsl" );
	resources.shader_particle	= shader_load( "dat/shaders/textured_phong.v.glsl",	"dat/shaders/textured_phong.f.glsl" );
	resources.shader_terrain	= shader_load( "dat/shaders/terrain.v.glsl",		"dat/shaders/terrain.f.glsl" );
	resources.shader_skybox		= shader_load( "dat/shaders/skybox.v.glsl",			"dat/shaders/skybox.f.glsl" );
	resources.shader_ui			= shader_load( "dat/shaders/ui.v.glsl",				"dat/shaders/ui.f.glsl" );

#define GET_UNIFORM_LOCATION( var ) \
	resources.uniforms.var = shader_findConstant( mhash( #var )); \
	assert( resources.uniforms.var != NULL );
	SHADER_UNIFORMS( GET_UNIFORM_LOCATION )
	VERTEX_ATTRIBS( VERTEX_ATTRIB_LOOKUP );
}

#define kMaxDrawCalls 256
/*
drawCall call_buffer[kMaxDrawCalls];
int next_call_index = 0;
*/

#define kCallBufferCount 8		// Needs to be at least as many as we have shaders
drawCall	call_buffer[kCallBufferCount][kMaxDrawCalls];
int			next_call_index[kCallBufferCount];

map* callbatch_map = NULL;
int callbatch_count = 0;


void render_clearCallBuffer( ) {
	memset( next_call_index, 0, sizeof( int ) * kCallBufferCount );
//	next_call_index = 0;
#if debug
	memset( call_buffer, 0, sizeof( drawCall ) * kMaxDrawCalls * kCallBufferCount );
	//memset( call_buffer, 0, sizeof( drawCall ) * kMaxDrawCalls );
#endif
}

#define		kRenderDrawBufferSize 1 * 1024 * 1024
uint8_t*	render_draw_buffer;
uint8_t*	render_draw_buffer_free;

// Temporary allocation that is only valid for a frame
void* render_bufferAlloc( size_t size ) {
	// TODO: Aligned!
	vAssert( ( render_draw_buffer_free + size ) < ( render_draw_buffer + kRenderDrawBufferSize ));
	void* p = render_draw_buffer_free;
	render_draw_buffer_free += size;
	return p;
}

void render_resetBufferBuffer() {
	render_draw_buffer_free = render_draw_buffer;
}
// Private Function declarations

void render_set3D( int w, int h ) {
	glViewport(0, 0, w, h);
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glDepthMask( GL_TRUE );
}

void render_set2D() {
	vAssert( 0 ); // NYI	
}

#ifdef ANDROID
void render_swapBuffers( egl_renderer* egl ) {
    eglSwapBuffers( egl->display, egl->surface );
}
#else
void render_swapBuffers() {
	glfwSwapBuffers(); // Send the 3d scene to the screen (flips display buffers)
	glFlush();
}
#endif // ANDROID

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s) {
	for (int i = 0; i < s->model_count; i++) {
		modelInstance_draw( scene_model( s, i ), s->cam );
	}
}

void render_lighting( scene* s ) {
	// Ambient Light

	// Point Lights	
	light_renderLights( s->light_count, s->lights );
}

// Clear information from last draw
void render_clear() {
	glClearColor( 1.f, 0.f, 0.0, 0.f );
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
	skybox_init();
	
	// Allocate space for buffers
	const GLsizei vertex_buffer_size = sizeof( vector ) * MAX_VERTEX_ARRAY_COUNT;
	const GLsizei element_buffer_size = sizeof( GLushort ) * MAX_VERTEX_ARRAY_COUNT;
	for ( int i = 0; i < kVboCount; i++ ) {
		resources.vertex_buffer[i]	= gl_bufferCreate( GL_ARRAY_BUFFER, NULL, vertex_buffer_size );
		resources.element_buffer[i]	= gl_bufferCreate( GL_ELEMENT_ARRAY_BUFFER, NULL, element_buffer_size );
	}

	// Allocate draw buffer
	render_draw_buffer = mem_alloc( kRenderDrawBufferSize );
	callbatch_map = map_create( kCallBufferCount, sizeof( unsigned int ));

// Now done in the main threadfunc due to separate android code path
//	render_initialised = true;
}

// Terminate the 3D rendering
void render_terminate() {
#ifndef ANDROID
	glfwTerminate();
#endif
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
void render( scene* s ) {
	render_clearCallBuffer();
	render_resetBufferBuffer();
	
	matrix_setIdentity( modelview );

	const float fov = 0.8f; // In radians
	const float aspect = 4.f/3.f;
	const float z_near = 1.f;
	const float z_far = 500.f;
	render_perspectiveMatrix( perspective, fov, aspect, z_near, z_far );

	camera* cam = s->cam;
	cam->near = z_near;
	cam->far = z_far;
	vector frustum[6];
	camera_calculateFrustum( cam, frustum );

	render_validateMatrix( cam->trans->world );
	matrix_inverse( camera_inverse, cam->trans->world );
	render_validateMatrix( modelview );
	render_resetModelView();

	render_lighting( s );

	render_scene( s );
}

int render_findDrawCallBuffer( shader* vshader ) {
	unsigned int key = (unsigned int)vshader;
	int* i_ptr = map_find( callbatch_map, key );
	int i;
	if ( !i_ptr ) {
		vAssert( callbatch_count < kCallBufferCount );
		map_add( callbatch_map, key, &callbatch_count );
		i = callbatch_count;
		++callbatch_count;
	} else {
		i = *i_ptr;
	}

	return i;
}

drawCall* drawCall_create( shader* vshader, int count, GLushort* elements, vertex* verts, GLint tex, matrix mv ) {

	// Lookup drawcall buffer from shader
	int call_buffer_index = render_findDrawCallBuffer( vshader );
	vAssert( next_call_index[call_buffer_index] < kMaxDrawCalls );
	drawCall* draw = &call_buffer[call_buffer_index][next_call_index[call_buffer_index]++];

//	drawCall* draw = &call_buffer[next_call_index++];
	draw->vitae_shader = vshader;
	draw->element_buffer = elements;
	draw->vertex_buffer = verts;
	draw->element_count = count;
	draw->texture = tex;
	matrix_cpy( draw->modelview, mv );
	return draw;
}

void render_drawCall( drawCall* draw ) {
	return;
}

int buffer_index = 0;

void render_drawCall_draw( drawCall* draw ) {
	// Copy our data to the GPU
	GLsizei vertex_buffer_size = draw->element_count * sizeof( vertex );
	GLsizei element_buffer_size = draw->element_count * sizeof( GLushort );

	VERTEX_ATTRIBS( VERTEX_ATTRIB_POINTER );
	// *** Vertex Buffer
	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer[buffer_index] );
	glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, draw->vertex_buffer, GL_DYNAMIC_DRAW );// OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW
	// *** Element Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer[buffer_index] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, draw->element_buffer, GL_DYNAMIC_DRAW ); // OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW

	// Draw!
	glDrawElements( GL_TRIANGLES, draw->element_count, GL_UNSIGNED_SHORT, (void*)0 );

//	glBindBuffer( GL_ARRAY_BUFFER, 0 );
//	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	buffer_index = ( buffer_index + 1 ) % kVboCount;
	VERTEX_ATTRIBS( VERTEX_ATTRIB_DISABLE_ARRAY )
}

void render_drawCall_internal( drawCall* draw ) {
	// For now, *always* write to depth buffer
//	glDepthMask( GL_TRUE );

//	shader_activate( draw->vitae_shader );
	
	// Textures
	render_setUniform_texture( *resources.uniforms.tex, draw->texture );

	// Set up uniform matrices
	render_setUniform_matrix( *resources.uniforms.modelview,	draw->modelview );
//	render_setUniform_matrix( *resources.uniforms.projection,	perspective );

	render_drawCall_draw( draw );
}

void render_drawCallBatch( int index ) {
	int count = next_call_index[index];
	if ( count > 0 ) {
		drawCall* draw = &call_buffer[index][0];
		// For now, *always* write to depth buffer
		glDepthMask( GL_TRUE );
		shader_activate( draw->vitae_shader );
		// Set up uniform matrices
		render_setUniform_matrix( *resources.uniforms.projection,	perspective );

//		VERTEX_ATTRIBS( VERTEX_ATTRIB_POINTER );
		//		render_drawCall_draw( draw );
		//		printf( "RENDER: Drawing callbatch %d (%d calls).\n", index, count );
		for ( int i = 0; i < count; i++ ) {
			render_drawCall_internal( &call_buffer[index][i] );
		}
//		VERTEX_ATTRIBS( VERTEX_ATTRIB_DISABLE_ARRAY )
	}
}

void render_draw( engine* e ) {
	vmutex_lock( &gl_mutex );
#ifdef ANDROID
	int w = 800;
#else
	int w = 640;
#endif
	int h = 480;
	render_set3D( w, h );
	render_clear();

	// Draw each batch of drawcalls
	for ( int i = 0; i < kCallBufferCount; i++ ) {
		render_drawCallBatch( i );	
	}
/*
	for ( int i = 0; i < next_call_index; i++ ) {
		render_drawCall_internal( &call_buffer[i] );
	}
	*/

#if ANDROID
	render_swapBuffers( e->egl );
#else
	render_swapBuffers();
#endif
	vmutex_unlock( &gl_mutex );
}

// threadsignal_render == 0 means that we are not ready to render
// threadsignal_render == 1 means that the engine thread has fully prepared and we can begin
void render_waitForEngineThread() {
	vthread_waitCondition( start_render );
}

void render_renderThreadTick( engine* e ) {
	PROFILE_BEGIN( PROFILE_RENDER_TICK );
	texture_tick();
	render_draw( e );
	// Indicate that we have finished
	threadsignal_render = 0;
	vthread_signalCondition( finished_render );
	PROFILE_END( PROFILE_RENDER_TICK );
}

//
// *** The Rendering Thread itself
//
void* render_renderThreadFunc( void* args ) {
	printf( "RENDER THREAD: Hello from the render thread!\n" );

#ifdef ANDROID
	struct android_app* app = args;
	engine* e = app->userData;
	e->egl = egl_init( app );
#else
	engine* e = args;
#endif
	render_init();
	render_initialised = true;
	printf( "RENDER THREAD: Render system initialised.\n");
	vthread_signalCondition( finished_render );

	while( true ) {
		render_waitForEngineThread();
		render_renderThreadTick( e );
	}

	return NULL;
}
