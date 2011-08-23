// engine.c
#include "src/common.h"
#include "src/engine.h"
//---------------------
#include "mem/allocator.h"
#include "font.h"
#include "input.h"
#include "lua.h"
#include "maths.h"
#include "model.h"
#include "particle.h"
#include "scene.h"
#include "transform.h"
#include "camera/flycam.h"
#include "render/debugdraw.h"
#include "render/modelinstance.h"
#include "render/render.h"
#include "debug/debug.h"
#include "debug/debugtext.h"
#include "system/file.h"

// Lua Libraries
#include <lauxlib.h>
#include <lualib.h>

// GLFW Libraries
#include <GL/glfw.h>

// System Libraries
#include <stdlib.h>


IMPLEMENT_LIST(delegate)

// System libraries

// *** Static Hacks
scene* theScene = NULL;
engine* static_engine_hack;
int w = 640, h = 480;

// Function Declarations
void engine_tickTickers( engine* e, float dt );
void engine_renderRenders( engine* e );
void engine_inputInputs( engine* e );
void engine_addTicker( engine* e, void* entity, tickfunc tick );

/*
 *
 *  Test Functions
 *
 */

void test_engine_init( engine* e ) {
#if 0
	debugtextframe* f = debugtextframe_create( 10.f, 10.f, 20.f );
	engine_addTicker( e, (void*)f, debugtextframe_tick );
	engine_addRender( e, (void*)f, debugtextframe_render );
	e->debugtext = f;
#endif
	

	theScene = test_scene_init( e );
//	theScene->debugtext = e->debugtext;
	lua_setScene( e->lua, theScene );

	particleEmitter* p = particleEmitter_create();
	p->lifetime = 4.f;
	p->size = property_create( 2 );
	property_addf( p->size, 0.f, 0.2f );
	property_addf( p->size, 2.f, 1.f );
	p->color = property_create( 5 );
	property_addv( p->color, 0.f, Vector( 1.f, 0.f, 0.f, 0.f ));
	property_addv( p->color, 0.3f, Vector( 1.f, 0.5f, 0.f, 1.f ));
	property_addv( p->color, 1.0f, Vector( 0.5f, 0.5f, 0.5f, 0.8f ));
	property_addv( p->color, 4.f, Vector( 0.5f, 0.5f, 0.5f, 0.f ));
	p->velocity = Vector( 0.f, 0.5f, 0.f, 0.f );
	p->spawn_interval = 0.1f;
	p->spawn_box = Vector( 0.3f, 0.f, 0.3f, 0.f );
	p->trans = transform_create();
	vector v = Vector( 2.f, 0.f, 2.f, 0.f );
	transform_setWorldSpacePosition( p->trans, &v );
	p->flags = p->flags | kParticleWorldSpace;
	engine_addRender( e, p, particleEmitter_render );
	startTick( e, p, particleEmitter_tick );
}

/*
 *
 *  Engine Methods
 *
 */

void engine_input( engine* e ) {
	scene_input( theScene, e->input );
	
	// Process all generic inputs
	engine_inputInputs( e );
}

// tick - process a frame of game update
void engine_tick( engine* e ) {
	float dt = timer_getDelta( e->timer );

	lua_preTick( dt );

	input_tick( e->input, dt );
	scene_tick( theScene, dt );

	engine_tickTickers( e, dt );

	if ( e->onTick && luaCallback_enabled( e->onTick ) ) {
#if DEBUG_LUA
		printf("Calling engine::onTick handler: %s\n", e->onTick->func);
#endif
		LUA_CALL( e->lua, e->onTick->func );
	}

}

// Static wrapper
void handleKeyPress(uchar key, int x, int y) {
	engine_handleKeyPress(static_engine_hack, key, x, y);
}

// Handle a key press from the user
void engine_handleKeyPress(engine* e, uchar key, int x, int y) {
	// Lua Test
	lua_getglobal(e->lua, "handleKeyPress");
	lua_pushnumber(e->lua, (int)key);
	lua_pcall(e->lua,	/* args */			1,
						/* returns */		1,
						/* error handler */ 0);
	int ret = lua_tonumber(e->lua, -1);
	printf("Lua says %d!\n", ret);

	switch (key) {
		case 27: // Escape key
			engine_terminate(e); // Exit the program
	}
}

// Handle a window resize
// Set the camera perspective and tell OpenGL how to convert 
// from coordinates to pixel values
void handleResize(int w_, int h_) {
	w = w_;
	h = h_;
}

// Initialise the Lua subsystem so that it is ready for use
void engine_initLua(engine* e, int argc, char** argv) {
	e->lua = vlua_create( e, "SpaceSim/lua/main.lua" );
}

// Create a new engine
engine* engine_create() {
	engine* e = mem_alloc(sizeof(engine));
	memset( e, 0, sizeof( engine ));
	e->timer = mem_alloc(sizeof(frame_timer));
	e->callbacks = luaInterface_create();
	e->onTick = luaInterface_addCallback(e->callbacks, "onTick");
	e->input = input_create();
	e->tickers = NULL;
	e->inputs = NULL;
	e->renders = NULL;
	return e;
}

// Initialise the engine
void engine_init(engine* e, int argc, char** argv) {

	timer_init(e->timer);

	// *** Init System
	rand_init();
	
	// *** Init Memory
	transform_initPool();
	modelInstance_initPool();

	// *** Initialise OpenGL
	render_init(e, argc, argv);

	// *** Start up Core Systems
	font_init();

	// *** Initialise Lua
	engine_initLua(e, argc, argv);
	luaInterface_registerCallback(e->callbacks, "onTick", "tick");

	// TEST
	test_engine_init( e );
}

// Initialises the application
void init(int argc, char** argv) {

	// *** Initialise Memory
	mem_init( argc, argv );
	debug_init( );

	// *** Static Module initialization
	scene_static_init();

	// *** Initialise Engine
	/*
	engine* e = engine_create();
	engine_init(e, argc, argv);
	static_engine_hack = e;
	*/

}

// terminateLua - terminates the Lua interpreter
void engine_terminateLua(engine* e) {
	lua_close(e->lua);
}

// terminate - terminates (De-initialises) the engine
void engine_terminate(engine* e) {
	// *** clean up Lua
	engine_terminateLua(e);

	// *** clean up Renderer
	render_terminate();

	exit(0);
}

// static_run
/*
void run() {
	engine_run( static_engine_hack );
}
*/

void engine_render( engine* e ) {
	render_set3D( w, h );
	render_clear();
	render( theScene, w, h );
	engine_renderRenders( e );
	glfwSwapBuffers(); // Send the 3d scene to the screen (flips display buffers)
}

// run - executes the main loop of the engine
void engine_run(engine* e) {
//	TextureLibrary* textures = texture_library_create();
	int running = true;
	handleResize(640, 480);	// Call once to init
	while (running) {
		engine_input( e );
		engine_tick( e );
		engine_render( e );
		running = !input_keyPressed( e->input, KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED );
	}
}

// Run through all the delegates, ticking each of them
void engine_tickTickers( engine* e, float dt ) {
	delegatelist* d = e->tickers;
	while (d != NULL) {
		assert( d->head );	// Should never be NULL heads
		delegate_tick( d->head, dt ); // tick the whole of this delegate
		d = d->tail;
	}
}

void engine_renderRenders( engine* e ) {
	delegatelist* d = e->renders;
	while (d != NULL) {
		assert( d->head );	// Should never be NULL heads
		delegate_render( d->head ); // render the whole of this delegate
		d = d->tail;
	}
}

void engine_inputInputs( engine* e ) {
	delegatelist* d = e->inputs;
	while ( d != NULL ) {
		assert( d->head );	// Should never be NULL heads
		delegate_input( d->head, e->input ); // render the whole of this delegate
		d = d->tail;
	}
}

// Search the list of delegates for one with the matching tick func
// and space to add a new entry
delegate* engine_findDelegate( delegatelist* d, void* func ) {
	while ( d != NULL ) {
		if ( d->head->tick == func && !delegate_isFull(d->head) ) {
			return d->head;
		}
		d = d->tail;
	}
	return NULL;
}

delegate* engine_findTickDelegate( engine* e, tickfunc tick ) {
	return engine_findDelegate( e->tickers, tick );
}

delegate* engine_findRenderDelegate( engine* e, renderfunc render ) {
	return engine_findDelegate( e->renders, render );
}

delegate* engine_findInputDelegate( engine* e, inputfunc in ) {
	return engine_findDelegate( e->inputs, in );
}
// Add a new delegate to the delegatelist
// (a delegate is a list of entities to all receive the same tick function)
// (the delegatelist contains all the delegates, ie. for each different tick function)


delegate* engine_addDelegate( delegatelist** d, void* func ) {
	delegatelist* dl = *d;
	if ( !*d ) {
//		*d = mem_alloc( sizeof( delegatelist ));
		*d = delegatelist_create();
		dl = *d;
	}
	else {
		while ( dl->tail != NULL)
			dl = dl->tail;
		dl->tail = delegatelist_create();
		dl = dl->tail;
	}
	dl->tail = NULL;
	dl->head = delegate_create( func, kDefaultDelegateSize );
	return dl->head;
}

delegate* engine_addTickDelegate( engine* e, tickfunc tick ) {
	return engine_addDelegate( &e->tickers, tick );
}

delegate* engine_addRenderDelegate( engine* e, renderfunc render ) {
	return engine_addDelegate( &e->renders, render );
}

delegate* engine_addInputDelegate( engine* e, inputfunc in ) {
	return engine_addDelegate( &e->inputs, in );
}

// Look for a delegate of the right type to add this entity too
// If one is not found, create one
void engine_addTicker( engine* e, void* entity, tickfunc tick ) {
	delegate* d = engine_findTickDelegate( e, tick );
	if ( !d )
		d = engine_addTickDelegate( e, tick );
	delegate_add( d, entity);
}
void startTick( engine* e, void* entity, tickfunc tick ) {
	engine_addTicker( e, entity, tick );
}

void startInput( engine* e, void* entity, inputfunc in ) {
	delegate* d = engine_findInputDelegate( e, in );
	if ( !d )
		d = engine_addInputDelegate( e, in );
	delegate_add( d, entity);
}

void engine_addRender( engine* e, void* entity, renderfunc render ) {
	delegate* d = engine_findRenderDelegate( e, render );
	if ( !d )
		d = engine_addRenderDelegate( e, render );
	delegate_add( d, entity );
}
