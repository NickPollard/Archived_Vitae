// engine.h
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "base/list.h"
#include "lua.h"
#include "ticker.h"
#include "time.h"

// Lua Libraries
#include <lua.h>

extern scene* theScene;

extern float depth;

DEF_LIST(delegate)
#define kDefaultdelegateSize 16

typedef struct {
	// *** General
	frame_timer* timer;
	input* input;

	// *** Lua
	lua_State* lua;					//!< Engine wide persistant lua state
	luaInterface* callbacks;		//!< Lua Interface for callbacks from the engine
	luaCallback* onTick;			//!< OnTick event handler

	delegatelist* tickers;	
	delegatelist* renders;

	debugtextframe* debugtext;
} engine;

/*
 *
 *  Static Functions
 *
 */

// init - initialises the application
void init(int argc, char** argv);

// run - executes the main loop of the application
void run();

/*
 *
 *  Engine Methods
 *
 */

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
void engine_addTicker( engine* e, void* entity, tickfunc tick );
void engine_addRender( engine* e, void* entity, renderfunc render );

void handleResize(int w, int h);

#endif // __ENGINE_H__
