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

#define MAX_VERTEX_ARRAY_COUNT 1024

// *** Shader Pipeline

matrix modelview, camera_inverse;
matrix perspective;

gl_resources resources;

const int kInitialWidth = 640;
const int kInitialHeight = 480;
#ifdef ANDROID
window window_main = { 800, 480 };
#else
window window_main = { 640, 480 };
#endif

GLuint render_glBufferCreate( GLenum target, const void* data, GLsizei size ) {
	GLuint buffer; // The OpenGL object handle we generate
	glGenBuffers( 1, &buffer );				// Generate a buffer name - effectively just a declaration
	glBindBuffer( target, buffer );			// Bind the buffer name to a target, creating the vertex buffer object
	// Usage hint can be: GL_[VARYING]_[USE]
	// Varying: STATIC / DYNAMIC / STREAM
	// Use: DRAW / READ / COPY
	// OpenGL ES only supports dynamic/static draw
	glBufferData( target, size, data, /*Usage hint*/ GL_STATIC_DRAW );	// Allocate the buffer, optionally copying data
//	printf( "render_glBufferCreate: Generated VBO name %u.\n", buffer );
	return buffer;
}

typedef struct bufferRequest_s {
	GLenum		target;
	const void*	data;
	GLsizei		size;
	GLuint*		ptr;
} bufferRequest;

#define	kMaxBufferRequests	128
bufferRequest	buffer_requests[kMaxBufferRequests];
int				buffer_request_count = 0;

// Mutex for buffer requests
vmutex buffer_mutex = kMutexInitialiser;

bufferRequest* getBufferRequest() {
	vAssert( buffer_request_count < kMaxBufferRequests );
	bufferRequest* r = &buffer_requests[buffer_request_count++];
	return r;
}

// Asynchronously create a VertexBufferObject
GLuint* render_requestBuffer( GLenum target, const void* data, GLsizei size ) {
//	printf( "RENDER: Buffer requested.\n" );
	bufferRequest* b = NULL;
	vmutex_lock( &buffer_mutex );
	{
		b = getBufferRequest();
		b->target	= target;
		b->data		= data;
		b->size		= size;
	}
	vmutex_unlock( &buffer_mutex );
	vAssert( b );
	// Needs to allocate a GLuint somewhere
	// and return a pointer to that
	b->ptr = mem_alloc( sizeof( GLuint ));
	return b->ptr;
}

// Load any waiting buffer requests
void render_bufferTick() {
	vmutex_lock( &buffer_mutex );
	{
		// TODO: This could be a lock-free queue
		for ( int i = 0; i < buffer_request_count; i++ ) {
			bufferRequest* b = &buffer_requests[i];
			*b->ptr = render_glBufferCreate( b->target, b->data, b->size );
		}
		buffer_request_count = 0;
	}
	vmutex_unlock( &buffer_mutex );
}

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
#define kCallBufferCount 8		// Needs to be at least as many as we have shaders
drawCall	call_buffer[kCallBufferCount][kMaxDrawCalls];
int			next_call_index[kCallBufferCount];

map* callbatch_map = NULL;
int callbatch_count = 0;


void render_clearCallBuffer( ) {
	memset( next_call_index, 0, sizeof( int ) * kCallBufferCount );
#if debug
	memset( call_buffer, 0, sizeof( drawCall ) * kMaxDrawCalls * kCallBufferCount );
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
	glClear(/* GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT );
}

// Handle a window resize
// Set the camera perspective and tell OpenGL how to convert 
// from coordinates to pixel values
void render_handleResize(int w, int h) {
	window_main.width = w;
	window_main.height = h;
}

// Initialise the window and the OpenGL buffers we need for rendering
void render_initWindow() {
#ifndef ANDROID
	if (!glfwInit())
		printf("ERROR - failed to init glfw.\n");

	glfwOpenWindow(kInitialWidth, kInitialHeight, 8, 8, 8, 8, 8, 0, GLFW_WINDOW);
	glfwSetWindowTitle("Vitae");
	glfwSetWindowSizeCallback(render_handleResize);
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
		resources.vertex_buffer[i]	= render_glBufferCreate( GL_ARRAY_BUFFER, NULL, vertex_buffer_size );
		resources.element_buffer[i]	= render_glBufferCreate( GL_ELEMENT_ARRAY_BUFFER, NULL, element_buffer_size );
	}

	// Allocate draw buffer
	render_draw_buffer = mem_alloc( kRenderDrawBufferSize );
	callbatch_map = map_create( kCallBufferCount, sizeof( unsigned int ));
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

	const float aspect = 4.f/3.f;

	camera* cam = s->cam;
	render_perspectiveMatrix( perspective, cam->fov, aspect, cam->z_near, cam->z_far );
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
	
	draw->vitae_shader = vshader;
	draw->element_buffer = elements;
	draw->vertex_buffer = verts;
	draw->element_count = count;
	draw->texture = tex;
	draw->element_buffer_offset = 0;
	draw->vertex_VBO	= resources.vertex_buffer[0];
	draw->element_VBO	= resources.element_buffer[0];
	draw->depth_mask = GL_TRUE;

	matrix_cpy( draw->modelview, mv );
	return draw;
}

void render_drawCall_draw( drawCall* draw ) {
	// Bind Correct buffers
	glBindBuffer( GL_ARRAY_BUFFER, draw->vertex_VBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, draw->element_VBO );
	
	// If required, copy our data to the GPU
	if ( draw->vertex_VBO == resources.vertex_buffer[0] ) {
		GLsizei vertex_buffer_size	= draw->element_count * sizeof( vertex );
		GLsizei element_buffer_size	= draw->element_count * sizeof( GLushort );
		glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, draw->vertex_buffer, GL_DYNAMIC_DRAW );// OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, draw->element_buffer, GL_DYNAMIC_DRAW ); // OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW
	}

	// Now Draw!
	VERTEX_ATTRIBS( VERTEX_ATTRIB_POINTER );
	glDrawElements( GL_TRIANGLES, draw->element_count, GL_UNSIGNED_SHORT, (void*)draw->element_buffer_offset );
	VERTEX_ATTRIBS( VERTEX_ATTRIB_DISABLE_ARRAY );
}

void render_drawBatch( drawCall* draw ) {
	render_setUniform_texture( *resources.uniforms.tex,			draw->texture );
	render_setUniform_matrix( *resources.uniforms.modelview,	draw->modelview );
	render_drawCall_draw( draw );
}

void render_drawCallBatch( int index ) {
	int count = next_call_index[index];
	if ( count > 0 ) {
		// For now, *always* write to depth buffer
		glDepthMask( call_buffer[index][0].depth_mask );
		shader_activate( call_buffer[index][0].vitae_shader );
		render_lighting( theScene );
		// Set up uniform matrices
		render_setUniform_matrix( *resources.uniforms.projection,	perspective );
		render_setUniform_matrix( *resources.uniforms.worldspace,	modelview );

		for ( int i = 0; i < count; i++ ) {
			render_drawBatch( &call_buffer[index][i] );
		}
	}
}

void render_draw( window* w, engine* e ) {
	(void)e;
#ifdef ANDROID
	w->width = 800;
	w->height = 480;
#endif
	render_set3D( w->width, w->height );
	render_clear();

	// Draw each batch of drawcalls
	for ( int i = 0; i < kCallBufferCount; i++ ) {
		render_drawCallBatch( i );	
	}

#if ANDROID
	render_swapBuffers( e->egl );
#else
	render_swapBuffers();
#endif
}

void render_waitForEngineThread() {
	vthread_waitCondition( start_render );
}

void render_renderThreadTick( engine* e ) {
	PROFILE_BEGIN( PROFILE_RENDER_TICK );
	texture_tick();
	render_resetModelView();
	render_draw( &window_main, e );
	// Indicate that we have finished
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
		render_bufferTick();
		render_waitForEngineThread();
		render_renderThreadTick( e );
	}

	return NULL;
}
