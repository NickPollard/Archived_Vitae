// engine.h
#pragma once

#include "base/list.h"
#include "lua.h"
#include "ticker.h"
#include "time.h"

#ifdef ANDROID
// Android Libraries
#include <android/log.h>
#include <android_native_app_glue.h>
// EGL
#include <EGL/egl.h>
#endif

// Lua Libraries
#include <lua.h>

#define DEBUG_LUA false

extern scene* theScene;

extern float depth;

extern int threadsignal_render;

DEF_LIST(delegate)
#define kDefaultDelegateSize 16

#ifdef ANDROID
typedef struct egl_renderer_s {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
} egl_renderer;
#endif

struct engine_s {
	// *** General
	frame_timer* timer;
	input* input;

	// *** Lua
	lua_State* lua;					//!< Engine wide persistant lua state
	luaInterface* callbacks;		//!< Lua Interface for callbacks from the engine
	luaCallback* onTick;			//!< OnTick event handler

	delegatelist* tickers;	
	delegatelist* renders;
	delegatelist* inputs;

	debugtextframe* debugtext;

	bool running;
	bool active;
#ifdef ANDROID
	egl_renderer* egl;
	struct android_app* app;
#endif
};

extern engine* static_engine_hack;

/*
 *
 *  Static Functions
 *
 */

// init - initialises the application
void init(int argc, char** argv);

/*
 *
 *  Engine Methods
 *
 */

// Create an engine
engine* engine_create();

// Initialise the engine
void engine_init(engine* e, int argc, char** argv);

// Initialise the OpenGL subsystem so it is ready for use
void engine_initOpenGL(engine* e, int argc, char** argv);

// Initialise the Lua subsystem so that it is ready for use
void engine_initLua(engine* e, int argc, char** argv);

// terminate - de-initialises the engine
void engine_terminate(engine* e);

// terminateLua - de-initialises the Lua interpreter
void engine_terminateLua(engine* e);

// run - executes the main loop of the application
void engine_run(engine* e);

// Tick the engine, processing a frame of game update
void engine_tick(engine* e);

// Handle a key press from the user
void engine_handleKeyPress(engine* e, uchar key, int x, int y);

// Look for a delegate of the right type to add this entity too
// If one is not found, create one
void startTick( engine* e, void* entity, tickfunc tick );

void startInput( engine* e, void* entity, inputfunc input );

void engine_addRender( engine* e, void* entity, renderfunc render );

void engine_removeRender( engine* e, void* entity, renderfunc render );

int array_find( void** array, int count, void* ptr );

void array_remove( void** array, int* count, void* ptr );
