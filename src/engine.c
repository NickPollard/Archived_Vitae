// engine.c
#include "src/common.h"
#include "src/engine.h"
//---------------------
#include "mem/allocator.h"
#include "collision.h"
#include "dynamicfog.h"
#include "font.h"
#include "input.h"
#include "lua.h"
#include "maths/maths.h"
#include "model.h"
#include "particle.h"
#include "scene.h"
#include "skybox.h"
#include "terrain.h"
#include "transform.h"
#include "camera/flycam.h"
#include "input/keyboard.h"
#include "render/debugdraw.h"
#include "render/modelinstance.h"
#include "render/render.h"
#include "render/texture.h"
#include "debug/debug.h"
#include "debug/debugtext.h"
#include "script/lisp.h"
#include "script/parse.h"
#include "system/thread.h"

// Lua Libraries
#include <lauxlib.h>
#include <lualib.h>

// System Libraries
#include <stdlib.h>

IMPLEMENT_LIST(delegate)

// System libraries

// *** Static Hacks
scene* theScene = NULL;
terrain* theTerrain = NULL;
#ifdef LINUX_X
xwindow xwindow_main = { NULL, 0x0, false };
#endif

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

	// Terrain
	{
		terrain* t = terrain_create();
		theTerrain = t;
		t->trans = transform_createAndAdd( theScene );
		terrain_setSize( t, 400.f, 800.f );
		terrain_setResolution( t, 20, 20 );

		engine_addRender( e, (void*)t, terrain_render );
		startTick( e, (void*)t, terrain_tick );
	}


	// Dynamic fog
	{
		dynamicFog* f = dynamicFog_create();
		f->scene = theScene;
		startTick( e, (void*)f, dynamicFog_tick );
	}
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

float frame_times[10];

void countVisibleParticleEmitters( engine* e ) {
	int count = 0;
	delegatelist* d_list = e->renders;
	while ( d_list ) {
		if ( d_list->head->tick == particleEmitter_render) {
			count += d_list->head->count;
		}
		d_list = d_list->tail;
	}
	printf( "Visible particle emitters: %d.\n", count );
}

// tick - process a frame of game update
void engine_tick( engine* e ) {
	PROFILE_BEGIN( PROFILE_ENGINE_TICK );
	float dt = timer_getDelta( e->timer );

	float time = 0.f;
	for ( int i = 0; i < 9; i++ ) {
		frame_times[i] = frame_times[i+1];
		time += frame_times[i];
	}
	frame_times[9] = dt;
	time += dt;
	time = time / 10.f;

	//printf( "TICK: frametime %.4fms (%.2f fps)\n", time, 1.f/time );

	debugdraw_preTick( dt );
	lua_preTick( e->lua, dt );

	input_tick( e->input, dt );
	scene_tick( theScene, dt );
	collision_tick( dt );

	engine_tickTickers( e, dt );

	//countVisibleParticleEmitters( e );

	if ( e->onTick && luaCallback_enabled( e->onTick ) ) {
#if DEBUG_LUA
		printf("Calling engine::onTick handler: %s\n", e->onTick->func);
#endif
		//LUA_CALL( e->lua, e->onTick->func );
	
		lua_getglobal( e->lua, e->onTick->func );				
		lua_pushnumber( e->lua, dt );
		lua_pcall( e->lua,	/* args */			1,
				/* returns */		0,
				/* error handler */ 0);
	}

	vector v = Vector( 0.0, 0.0, 30.0, 1.0 );
	theTerrain->sample_point = matrix_vecMul( theScene->cam->trans->world, &v );
	PROFILE_END( PROFILE_ENGINE_TICK );

}

// Handle a key press from the user
void engine_handleKeyPress(engine* e, uchar key, int x, int y) {
	(void)x;
	(void)y;
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

// Initialise the Lua subsystem so that it is ready for use
void engine_initLua(engine* e, int argc, char** argv) {
	(void)argc;
	(void)argv;
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

	e->running = true;

	timer_init(e->timer);

	// *** Init System
	rand_init();
	vfile_systemInit();

	// *** Initialise OpenGL
	// On Android this is done in response to window events
	// see Android.c
#ifndef ANDROID
	//render_init();
	// Kick off rendering thread
	// Pass a pointer to the engine as args
	vthread render_thread = vthread_create( render_renderThreadFunc, (void*)e );
	(void)render_thread;
	// Wait for initialization to finish
	while ( !render_initialised ) {
//		printf( "blarg.\n" );
		vthread_waitCondition( finished_render );
//		sleep( 0 );
	}
#endif // ANDROID

	// *** Start up Core Systems
	particle_init();
	//font_init();

	// *** Initialise Lua
	engine_initLua(e, argc, argv);
	luaInterface_registerCallback(e->callbacks, "onTick", "tick");

	// TEST
	test_engine_init( e );
}

// Initialises the application
// Static Initialization should happen here
void init(int argc, char** argv) {

	// *** Initialise Memory
	mem_init( argc, argv );
	// Pools
	transform_initPool();
	modelInstance_initPool();

	// *** Static Module initialization
	scene_initStatic();
	lisp_init();
	parse_init();
	collision_init();
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

void engine_waitForRenderThread() {
	PROFILE_BEGIN( PROFILE_ENGINE_WAIT );
	vthread_waitCondition( finished_render );
	PROFILE_END( PROFILE_ENGINE_WAIT );
}

void engine_render( engine* e ) {
	PROFILE_BEGIN( PROFILE_ENGINE_RENDER );
#ifdef ANDROID
	if ( window_main.context != 0 ) {
		render( theScene );
		skybox_render( NULL );
		engine_renderRenders( e );
	}
#else
	render( theScene );
	skybox_render( NULL );
	engine_renderRenders( e );
#endif // ANDROID
	// Allow the render thread to start
	vthread_signalCondition( start_render );
	PROFILE_END( PROFILE_ENGINE_RENDER );
}

#ifdef LINUX_X
void engine_xwindowPollEvents( engine* e ) {
	XEvent event;
	if ( XPending( xwindow_main.display ) > 0 ) {
		XNextEvent( xwindow_main.display, &event );
		if ( event.type == DestroyNotify ) {
			// This message means we received our own message to destroy the window, so we clean up
			xwindow_main.open = false;
		}
		if ( event.type == ClientMessage ) {
			// This message means the user has asked to close the window, so we send the message
			XDestroyWindow( xwindow_main.display, xwindow_main.window );
		}
		if ( event.type == KeyPress ) {
			input_xKeyPress( event.xkey.keycode );
		}
		if ( event.type == KeyRelease ) {
			input_xKeyRelease( event.xkey.keycode );
		}
	}

	(void)e;
}
#endif // LINUX_X

#ifdef ANDROID
void engine_androidPollEvents( engine* e ) {       
//	printf( "Polling for android events." );
	// Read all pending events.
	int ident;
	int events;
	struct android_poll_source* source;

	// If not animating, we will block forever waiting for events.
	// If animating, we loop until all events are read, then continue
	// to draw the next frame of animation.
	while (( ident = ALooper_pollAll(0, NULL, &events, (void**)&source )) >= 0) {
		// Process this event.
		if (source != NULL) {
			source->process( e->app, source );
		}

		// Check if we are exiting.
		if ( e->app->destroyRequested != 0 ) {
			e->running = false;
			return;
		}
	}
}
#endif // ANDROID

// run - executes the main loop of the engine
void engine_run(engine* e) {
#if PROFILE_ENABLE
	profile_init();
#endif
	PROFILE_BEGIN( PROFILE_MAIN );
	//	TextureLibrary* textures = texture_library_create();

	// We need to ensure we can run the first time without waiting for render
	vthread_signalCondition( finished_render );
	
	while ( e->running ) {
#ifdef ANDROID
		engine_androidPollEvents( e );
		bool active = e->active;
#else
#ifdef LINUX_X
		engine_xwindowPollEvents( e );
#endif
		bool active = true;
#endif // ANDROID
		if ( active ) {
			engine_input( e );
			engine_tick( e );
			engine_waitForRenderThread();
			engine_render( e );
			e->running = e->running && !input_keyPressed( e->input, KEY_ESC );
		}
		bool window_open = true;
#ifdef LINUX_X
		window_open = xwindow_main.open;
#endif // LINUX_X
		e->running = e->running && window_open;

#if PROFILE_ENABLE
		profile_newFrame();
#endif
	}
	PROFILE_END( PROFILE_MAIN );
#if PROFILE_ENABLE
	profile_newFrame();
	profile_dumpProfileTimes();
#endif
}

// Run through all the delegates, ticking each of them
void engine_tickTickers( engine* e, float dt ) {
	delegatelist* d = e->tickers;
	while (d != NULL) {
		assert( d->head );	// Should never be NULL heads
		delegate_tick( d->head, dt, e ); // tick the whole of this delegate
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

void engine_removeDelegateEntry( delegatelist* d, void* entity, void* delegate_func ) {
	delegatelist* d_list = d;
	while ( d_list ) {
		if ( d_list->head->tick == delegate_func ) {
			delegate_remove( d_list->head, entity );
		}
		d_list = d_list->tail;
	}
}

void startTick( engine* e, void* entity, tickfunc tick ) {
	engine_addTicker( e, entity, tick );
}
void stopTick( engine* e, void* entity, tickfunc tick ) {
	engine_removeDelegateEntry( e->tickers, entity, tick );
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

void engine_removeRender( engine* e, void* entity, renderfunc render ) {
	engine_removeDelegateEntry( e->renders, entity, render );
}


int array_find( void** array, int count, void* ptr ) {
	for ( int i = 0; i < count; ++i ) {
		if ( array[i] == ptr )
			return i;
	}
	return -1;
}

void array_add( void** array, int* count, void* ptr ) {
	array[(*count)++] = ptr;
}

void array_remove( void** array, int* count, void* ptr ) {
	int i = array_find( array, *count, ptr );
	if ( i != -1 ) {
		--(*count);
		array[i] = array[*count];
		array[*count] = NULL;
	}
}

// TODO - move array funcs out and unit-test (0, negative, out of bounds, end of array)
